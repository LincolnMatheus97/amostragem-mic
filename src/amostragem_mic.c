#include <stdio.h>
#include "pico/stdlib.h"
#include "display.h"
#include "dma.h"
#include "mic.h"

int main()
{
    stdio_init_all();
    init_barr_i2c(); // Inicializa a barramento I2C para o display OLED
    init_display(); // Inicializa o display OLED
    
    clear_display();
    char boas_vindas[20];
    snprintf(boas_vindas, sizeof(boas_vindas), "INICIANDO SISTEMA");
    draw_display(10, 32, 1, boas_vindas);
    show_display();
    sleep_ms(3000);

    clear_display();
    init_config_adc();
    init_config_dma();
    char init_mic[20];
    snprintf(init_mic, sizeof(init_mic), "CONFIGURANDO O MIC");
    draw_display(10, 32, 1, init_mic);
    show_display();
    
    sleep_ms(4000);

    while (1)
    {
        sample_mic(); // Amostra o microfone usando DMA

        float voltage_rms = get_voltage_rms(); // Obtém a tensão RMS do microfone
        float db_level = get_db_simulated(voltage_rms); // Calcula o nível relativo de dB a partir da tensão RMS
        const char* sound_level = classify_sound_level(db_level); // Classifica o nível sonoro

        printf("Debug: O nível de dB é %.1f\n", db_level); 

        clear_display();

        char title[30];
        snprintf(title, sizeof(title), "Intensidade Sonora");
        draw_display(10, 0, 1, title);

        char db_val[30];
        snprintf(db_val, sizeof(db_val), "Nivel: %.1f dB", db_level); 
        draw_display(0, 16, 1, db_val);
        
        char nivel_desc[30];
        char aviso_extra[30] = ""; 

        if (sound_level == "Baixo") {
            snprintf(nivel_desc, sizeof(nivel_desc), "Classif. Baixo");
            snprintf(aviso_extra, sizeof(aviso_extra), "Nivel Seguro!");
        } else if (sound_level == "Moderado") {
            snprintf(nivel_desc, sizeof(nivel_desc), "Classif. Moderado");
            snprintf(aviso_extra, sizeof(aviso_extra), "Cuidado com o tempo!");
        } else {
            snprintf(nivel_desc, sizeof(nivel_desc), "Classif. Alto");
            snprintf(aviso_extra, sizeof(aviso_extra), "Nivel Prejudicial!");
        }

        draw_display(0, 32, 1, nivel_desc);

        if (aviso_extra[0] != '\0') { // Se a string de aviso não estiver vazia
            draw_display(0, 48, 1, aviso_extra);
        }

        show_display();
        
        sleep_ms(100); 
    }

    return 0;
}
