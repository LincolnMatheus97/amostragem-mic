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

// --- Variáveis globais para comunicação entre os núcleos ---
volatile float g_db_level = 0.0f;
volatile int g_qnt_leds_acessos = 0;
volatile const char* g_sound_level_str = "Iniciando";
volatile bool g_is_alarm_active = false; // Indica se o alarme (sonoro e visual) está ativo
volatile bool g_pause_buzzer_for_render = false; // Flag para cooperação entre núcleos

// --- NÚCLEO 1: APENAS INTERFACE DE USUÁRIO ---
void core1_entry() {
    inicializar_matriz();
    init_barr_i2c();
    init_display();

    clear_display();
    draw_display(10, 32, 1, "INICIANDO SISTEMA");
    show_display();
    sleep_ms(2000);
    clear_display();
    draw_display(10, 32, 1, "SISTEMA ATIVO");
    show_display();
    sleep_ms(2000);

    char sound_level_local[16];
    while (1) {
        // Copia os dados globais para variáveis locais para evitar condições de corrida
        float db_level_local = g_db_level;
        int leds_local = g_qnt_leds_acessos;
        strcpy(sound_level_local, (const char *)g_sound_level_str);

        // A matriz de LED SEMPRE reflete o nível de som atual, sem piscar
        atualizar_ledbar(leds_local);

        // --- LÓGICA DE COOPERAÇÃO ---
        // Se o alarme sonoro estiver ativo, peça ao Núcleo 0 para pausar o buzzer brevemente
        if (g_is_alarm_active) {
            g_pause_buzzer_for_render = true;
            busy_wait_us_32(100); // Um delay mínimo para garantir que o Núcleo 0 veja a flag
        }

        renderizar(); // A operação crítica que causa o conflito (atualiza a matriz via PIO)

        // Libera o buzzer imediatamente após a atualização da matriz
        if (g_is_alarm_active) {
            g_pause_buzzer_for_render = false;
        }
        
        // O display OLED continua atualizando normalmente
        display_update(db_level_local, sound_level_local);

        sleep_ms(30);
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

    // Variáveis para a lógica de alarme sustentado
    uint64_t high_level_start_time = 0;
    bool high_level_timer_running = false;
    const uint64_t ALARME_SUSTENTADO_US = TEMPO_ALARME_SUSTENTADO_S * 1000 * 1000;

    while (1)
    {
        sample_mic();
        float voltage_rms = get_voltage_rms();
        float db_level = get_db_simulated(voltage_rms);
        const char* sound_level = classify_sound_level(db_level);

        // Atualiza as variáveis globais para o Núcleo 1
        g_db_level = db_level;
        g_sound_level_str = sound_level;
        
        int qnt_leds_acessos = (int)((db_level - MIN_DB_MAPA) / (MAX_DB_MAPA - MIN_DB_MAPA) * QTD_LEDS);
        
        if (qnt_leds_acessos > QTD_LEDS) {
            qnt_leds_acessos = QTD_LEDS;
        } else if (qnt_leds_acessos < 0) {
           qnt_leds_acessos = 0; 
        } 
        
        g_qnt_leds_acessos = qnt_leds_acessos;

        // --- LÓGICA DE ALARME SUSTENTADO ---
        if (strcmp(sound_level, "Alto") == 0) {
            // Se o som está alto e o timer não foi iniciado, inicie-o
            if (!high_level_timer_running) {
                high_level_start_time = time_us_64();
                high_level_timer_running = true;
                printf("Nivel alto detectado. Iniciando contagem para alarme...\n");
            }
            // Se o timer está rodando e o tempo foi atingido, ative o alarme
            else if (time_us_64() - high_level_start_time > ALARME_SUSTENTADO_US) {
                if (!g_is_alarm_active) {
                    printf("ALARME ATIVADO: Nivel alto sustentado!\n");
                }
                g_is_alarm_active = true;
            }
        } else {
            // Se o som não está mais alto, resete tudo
            high_level_timer_running = false;
            if (g_is_alarm_active) {
                printf("Alarme desativado. Nivel de ruido normalizado.\n");
            }
            g_is_alarm_active = false;
        }

        // --- CONTROLE DO BUZZER ---
        if (g_is_alarm_active && !g_pause_buzzer_for_render) {
            set_pwm_buzzer(PINO_BUZZER, 3000, 50); // Liga o buzzer
        } else {
            set_pwm_buzzer(PINO_BUZZER, 0, 0); // Desliga o buzzer (seja por pausa ou alarme inativo)
        }

        printf("Debug Nucleo 0: dB: %.1f, Alarm Active: %d, Paused: %d\n", g_db_level, g_is_alarm_active, g_pause_buzzer_for_render);
        sleep_ms(50); 
    }

    return 0;
}