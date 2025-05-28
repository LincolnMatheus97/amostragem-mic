#include <stdio.h>
#include "pico/stdlib.h"
#include "display.h"
#include "dma.h"
#include "mic.h"

#define abs(x) ((x < 0) ? (-x) : (x))

int main()
{
    stdio_init_all();
    init_barr_i2c();
    init_display();
    
    clear_display();
    char boas_vindas[20];
    snprintf(boas_vindas, sizeof(boas_vindas), "INICIANDO SISTEMA");
    draw_display(10, 32, 2, boas_vindas);
    show_display();
    sleep_ms(3000);

    clear_display();
    init_config_adc();
    char init_adc[20];
    snprintf(init_adc, sizeof(init_adc), "CONFIGURANDO ADC");
    draw_display(10, 32, 2, init_adc);
    show_display();
    sleep_ms(2000);

    clear_display();
    init_config_dma();
    char init_dma[20];
    snprintf(init_dma, sizeof(init_dma), "CONFIGURANDO DMA");
    draw_display(10, 32, 2, init_dma);
    show_display();
    
    sleep_ms(5000);

    while (1)
    {
        sample_mic();

        float avg = mic_power();
        avg = 2.f * abs(ADC_ADJUST(avg));

        uint intensity = get_intensity(avg);

        clear_display();

        char titule[30];
        snprintf(titule, sizeof(titule), "Intensidade Sonora");
        draw_display(0, 0, 1, titule);
        show_display();

        char leitura_som[30];
        snprintf(leitura_som, sizeof(leitura_som), "Leitura: %2n %8.4f", intensity, avg);
        draw_display(0, 32, 1, leitura_som);
        show_display();
        
        sleep_ms(500);
    }

    return 0;
}
