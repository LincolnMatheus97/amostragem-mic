#ifndef MIC_H
#define MIC_H

#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "dma.h"

// Pino e canal do microfone no ADC.
#define MIC_CHANNEL 2
#define MIC_PIN (26 + MIC_CHANNEL)

// Parâmetros e macros do ADC.
#define ADC_CLOCK_DIV 96.f
#define SAMPLES 2500 
#define ADC_MAX 3.3f
#define ADC_STEP (3.3f/5.f)

// Constantes para cálculo de dB (decibeis)
#define V_REF_DB 0.00004f               // Tensão de referência para cálculo de dB (40uV RMS)
#define MIN_V_RMS_FOR_DB 0.000001f      // Tensão RMS mínima para cálculo de dB (1uV RMS)
#define MIN_DB_DISPLAY_LEVEL 30.0f      // Nível mínimo de dB a ser exibido

// Limiares para classificação de nível sonoro (ajuste conforme necessário)
#define LIMIAR_DB_BAIXO_MODERADO 60.0f
#define LIMIAR_DB_MODERADO_ALTO 80.0f

void init_config_adc();
void sample_mic();
float get_voltage_rms();
float get_db_simulated(float voltage_rms);
const char* classify_sound_level(float db_level);

#endif