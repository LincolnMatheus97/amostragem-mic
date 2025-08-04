#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/timer.h"
#include "display.h"
#include "dma.h"
#include "mic.h"
#include "matriz.h"
#include "buzzer.h"

// Variáveis globais para comunicação entre os núcleos
volatile float g_db_level = 0.0f;
volatile int g_qnt_leds_acessos = 0;
volatile const char* g_sound_level_str = "Iniciando";
volatile bool g_alarm_beeping = false;

// --- NÚCLEO 1: APENAS INTERFACE DE USUÁRIO ---
void core1_entry() {
    // 1. Inicializa APENAS o hardware da interface (Display e Matriz)
    inicializar_matriz();
    init_barr_i2c();
    init_display();

    // 2. Mostra as mensagens de boas-vindas
    clear_display();
    draw_display(10, 32, 1, "INICIANDO SISTEMA");
    show_display();
    sleep_ms(2000);

    clear_display();
    draw_display(5, 28, 1, "SISTEMA ATIVO");
    show_display();
    sleep_ms(2000);

    // 3. Loop infinito de atualização da UI
    char sound_level_local[16];
    while (1) {
        float db_level_local = g_db_level;
        int leds_local = g_qnt_leds_acessos;
        strcpy(sound_level_local, (const char *)g_sound_level_str);

        if (!g_alarm_beeping) {
            atualizar_ledbar(leds_local);
            renderizar();
        }
        display_update(db_level_local, sound_level_local);

        sleep_ms(50);
    }
}

// --- NÚCLEO 0: APENAS LÓGICA E PROCESSAMENTO ---
int main()
{
    stdio_init_all();
    
    inicializar_pwm_buzzer(PINO_BUZZER);
    init_config_adc();
    init_config_dma(); 

    printf("Lancando interface no Nucleo 1...\n");
    multicore_launch_core1(core1_entry);
    
    sleep_ms(4000); 

    // --- MUDANÇA: Nova máquina de estados para o alarme ---
    enum AlarmState { IDLE, BEEPING, COOLDOWN };
    enum AlarmState alarm_state = IDLE;

    uint64_t alarme_start_time = 0;
    const uint64_t ALARME_DURACAO_US = 1000 * 1000; // 1 segundo em microssegundos

    while (1)
    {
        sample_mic();
        float voltage_rms = get_voltage_rms();
        float db_level = get_db_simulated(voltage_rms);
        const char* sound_level = classify_sound_level(db_level);

        g_db_level = db_level;
        g_sound_level_str = sound_level;
        
        int qnt_leds_acessos = (int)((db_level - MIN_DB_MAPA) / (MAX_DB_MAPA - MIN_DB_MAPA) * QTD_LEDS);
        if (qnt_leds_acessos > QTD_LEDS) {
            qnt_leds_acessos = QTD_LEDS;
        } else if(qnt_leds_acessos < 1) {
            qnt_leds_acessos = 1;
        }
        g_qnt_leds_acessos = qnt_leds_acessos;

        // --- MÁQUINA DE ESTADOS DO ALARME ---
        switch (alarm_state) {
            case IDLE:
                // Se o som ficar alto, começa a apitar.
                if (strcmp(sound_level, "Alto") == 0) {
                    printf("ALERTA: Ruido alto detectado. Iniciando beep.\n");
                    // Para deixar o som mais alto, aumente a frequência. Tente 2000, 3000, etc.
                    set_pwm_buzzer(PINO_BUZZER, 3000, 50); // Som mais agudo e alto
                    alarme_start_time = time_us_64();
                    alarm_state = BEEPING;
                }
                break;

            case BEEPING:
                // Se o beep já tocou por tempo suficiente, para e entra em cooldown.
                if (time_us_64() - alarme_start_time > ALARME_DURACAO_US) {
                    printf("Beep de 1s finalizado. Entrando em modo Cooldown.\n");
                    set_pwm_buzzer(PINO_BUZZER, 0, 0); // Desliga o buzzer
                    alarm_state = COOLDOWN;
                }
                break;
            
            case COOLDOWN:
                // Fica neste estado (com o buzzer desligado) até o som baixar.
                // Isso evita que o alarme toque repetidamente.
                if (strcmp(sound_level, "Alto") != 0) {
                    printf("Ruido normalizado. Sistema de alerta rearmado.\n");
                    alarm_state = IDLE;
                }
                break;
        }

        printf("Debug Nucleo 0: dB: %.1f, Class: %s, AlarmState: %d\n", g_db_level, g_sound_level_str, alarm_state);
        sleep_ms(100); 
    }

    return 0;
}