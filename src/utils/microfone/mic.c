#include "mic.h"

void config_adc()
{
    adc_gpio_init(MIC_PIN);
    adc_init();
    adc_select_input(MIC_CHANNEL);

    adc_fifo_setup(
        true,
        true,
        1,
        false,
        false
    );

    adc_set_clkdiv(ADC_CLOCK_DIV);
}

void sample_mic()
{
    adc_fifo_drain();
    adc_run(false);
    
    dma_channel_configure(dma_channel, &dma_cfg,
        adc_buffer,
        &(adc_hw->fifo),
        SAMPLES,
        true
    );

    adc_run(true);
    dma_channel_wait_for_finish_blocking(dma_channel);

    adc_run(false);
}

float mic_power() 
{
    float avg = 0.f;

    for (uint i = 0; i < SAMPLES; i++)
    {
        avg += adc_buffer[i] * adc_buffer[i];
    }
    
    avg /= SAMPLES;
    return sqrt(avg);
}

uint8_t get_intensity(float v)
{
    uint count = 0;

    while ((v -= ADC_STEP/20) > 0.f) 
    {
        count++;
    }

    return count;
}