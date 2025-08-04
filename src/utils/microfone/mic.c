#include "mic.h"

uint16_t adc_buffer[SAMPLES];

void init_config_adc() // Inicializa o ADC (Conversor Analógico-Digital) para o microfone
{
    adc_gpio_init(MIC_PIN);
    adc_init();
    adc_select_input(MIC_CHANNEL);

    adc_fifo_setup(
        true,
        true,
        1,
        false,
        false);

    adc_set_clkdiv(ADC_CLOCK_DIV);
}

void sample_mic() // Amostra o microfone usando DMA
{
    adc_fifo_drain(); // Limpa o FIFO do ADC antes de iniciar a amostragem
    adc_run(false);   // Desliga o ADC antes de configurar o DMA

    dma_channel_configure(dma_channel, &dma_cfg,
                          adc_buffer,
                          &(adc_hw->fifo),
                          SAMPLES,
                          true); // Configura o canal DMA para transferir dados do ADC para o buffer adc_buffer

    adc_run(true); // Liga o ADC para iniciar a amostragem
    dma_channel_wait_for_finish_blocking(dma_channel);

    adc_run(false);
}

// Obtém a tensão RMS (Root Mean Square) do microfone
// A tensão RMS é uma medida da tensão efetiva de uma onda AC (Corrente Alternada).
// A tensão RMS é calculada a partir das amostras digitais do ADC, subtraindo o offset DC (Corrente Continua) e calculando a média dos quadrados das amostras AC.
float get_voltage_rms()
{
    if (SAMPLES == 0)
        return 0.0f;

    float dc_offset_digital = 0.0f;
    for (uint i = 0; i < SAMPLES; i++)
        dc_offset_digital += adc_buffer[i];
    dc_offset_digital /= SAMPLES;

    float sum_sq_ac_digital = 0.0f;
    for (uint i = 0; i < SAMPLES; i++) {
        float ac_sample = (float)adc_buffer[i] - dc_offset_digital;
        sum_sq_ac_digital += ac_sample * ac_sample;
    }

    float rms_ac_digital = sqrtf(sum_sq_ac_digital / SAMPLES);

    if (rms_ac_digital < 5.0f) // filtro de ruído
        return 0.0f;

    float voltage_rms = rms_ac_digital * (3.3f / 4096.0f);

    // suavização
    // static float last_voltage_rms = 0.0f;
    // voltage_rms = 0.5f * last_voltage_rms + 0.5f * voltage_rms;
    // last_voltage_rms = voltage_rms;

    return voltage_rms;
}

// Calcula o nível de dB (decibéis) a partir da tensão RMS do microfone.
// O nível de dB é calculado usando a fórmula: dB = 20 * log10(V / V_REF_DB)
// Onde V é a tensão RMS do microfone e V_REF_DB é a tensão de referência para o cálculo de dB (20uV RMS).
float get_db_simulated(float voltage_rms)
{
    float db_level = 0.0f;

    if (voltage_rms < MIN_V_RMS_FOR_DB)
    {
        db_level = MIN_DB_DISPLAY_LEVEL;
    }
    else
    {
        db_level = 73.0f * log10f(voltage_rms / V_REF_DB) - 40.0f;
    }

    if (db_level < MIN_DB_DISPLAY_LEVEL)
    {
        db_level = MIN_DB_DISPLAY_LEVEL;
    }
    if (db_level > 120.0f)
    {
        db_level = 120.0f;
    }

    return db_level;
}

const char* classify_sound_level(float db_level)
{   
    const char* level_description;
    if (db_level < LIMIAR_DB_BAIXO_MODERADO)
    {
        level_description = "Baixo";
    }
    else if (db_level < LIMIAR_DB_MODERADO_ALTO)
    {
        level_description = "Moderado";
    }
    else
    {
        level_description = "Alto";
    }

    return level_description;
}