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
    // ETAPA 1: PARADA E LIMPEZA GERAL
    // Garante que qualquer operação anterior seja completamente interrompida para evitar conflitos.
    dma_channel_abort(dma_channel);
    adc_run(false);

    // Drena o FIFO do ADC para remover quaisquer dados antigos ou corrompidos.
    adc_fifo_drain();

    // LIMPA A FLAG DE ERRO DE OVERFLOW. Esta é a correção mais importante para o seu problema.
    // Se um som muito alto causou um erro, esta linha o reseta.
    adc_hw->fcs |= ADC_FCS_OVER_BITS;


    // ETAPA 2: RECONFIGURAÇÃO E PARTIDA
    // Reconfigura o DMA para começar a escrever no início do nosso buffer.
    dma_channel_configure(
        dma_channel,
        &dma_cfg,
        adc_buffer,           // Ponteiro para o buffer de destino
        &adc_hw->fifo,          // Ponteiro para a fonte (ADC FIFO)
        SAMPLES,              // Quantidade de amostras a capturar
        true                    // Inicia a transferência imediatamente
    );

    // Liga a amostragem contínua do ADC
    adc_run(true);


    // ETAPA 3: ESPERA SINCRONIZADA
    // Pausa o Núcleo 0 aqui até o DMA confirmar que preencheu todo o buffer.
    // Isso garante que nunca processaremos dados incompletos.
    dma_channel_wait_for_finish_blocking(dma_channel);
}

// Obtém a tensão RMS (Root Mean Square) do microfone
// A tensão RMS é uma medida da tensão efetiva de uma onda AC (Corrente Alternada).
// A tensão RMS é calculada a partir das amostras digitais do ADC, subtraindo o offset DC (Corrente Continua) e calculando a média dos quadrados das amostras AC.
float get_voltage_rms()
{
    if (SAMPLES == 0) return 0.0f;

    // Calcula a média das amostras para encontrar o nível DC (offset)
    float dc_offset_digital = 0.0f;
    for (uint i = 0; i < SAMPLES; i++)
        dc_offset_digital += adc_buffer[i];
    dc_offset_digital /= SAMPLES;

    // Calcula a soma dos quadrados da componente AC (amostra - offset DC)
    float sum_sq_ac_digital = 0.0f;
    for (uint i = 0; i < SAMPLES; i++) {
        float ac_sample = (float)adc_buffer[i] - dc_offset_digital;
        sum_sq_ac_digital += ac_sample * ac_sample;
    }

    // Calcula o valor RMS digital (raiz da média dos quadrados)
    float rms_ac_digital = sqrtf(sum_sq_ac_digital / SAMPLES);

    // Converte o valor RMS digital para uma tensão real em Volts
    float voltage_rms = rms_ac_digital * (3.3f / 4096.0f);
    
    return voltage_rms; // Retorna a tensão RMS "pura" desta amostragem
}

// Calcula o nível de dB (decibéis) a partir da tensão RMS do microfone.
// O nível de dB é calculado usando a fórmula: dB = 20 * log10(V / V_REF_DB)
// Onde V é a tensão RMS do microfone e V_REF_DB é a tensão de referência para o cálculo de dB (20uV RMS).
float get_db_simulated(float voltage_rms)
{
    float db_level_cru;

    // Passo 1: Calcula o dB "cru" a partir da tensão.
    if (voltage_rms < MIN_V_RMS_FOR_DB) {
        db_level_cru = MIN_DB_DISPLAY_LEVEL;
    } else {
        db_level_cru = 73.0f * log10f(voltage_rms / V_REF_DB) - 40.0f;
    }

    // Garante que o valor bruto não seja menor que o piso antes de processar.
    if (db_level_cru < MIN_DB_DISPLAY_LEVEL) {
        db_level_cru = MIN_DB_DISPLAY_LEVEL;
    }

    // Passo 2: Suavização com ATAQUE RÁPIDO e DECAIMENTO LENTO.
    static float db_level_suavizado = MIN_DB_DISPLAY_LEVEL;
    
    if (db_level_cru > db_level_suavizado) {
        // ATAQUE RÁPIDO: Se o som novo é mais alto, a barra sobe rapidamente.
        db_level_suavizado = 0.40f * db_level_suavizado + 0.60f * db_level_cru;
    } else {
        // DECAIMENTO LENTO: Se o som está diminuindo, a barra desce suavemente.
        db_level_suavizado = 0.96f * db_level_suavizado + 0.04f * db_level_cru;
    }

    // Passo 3: Aplica os limites finais.
    float db_level_final = db_level_suavizado;
    if (db_level_final < MIN_DB_DISPLAY_LEVEL) {
        db_level_final = MIN_DB_DISPLAY_LEVEL;
    }
    if (db_level_final > 120.0f) {
        db_level_final = 120.0f;
    }

    return db_level_final;
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