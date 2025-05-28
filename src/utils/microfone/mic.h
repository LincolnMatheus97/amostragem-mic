#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "dma.h"

// Pino e canal do microfone no ADC.
#define MIC_CHANNEL 2
#define MIC_PIN (26 + MIC_CHANNEL)

// Parâmetros e macros do ADC.
#define ADC_CLOCK_DIV 96.f
#define SAMPLES 500 
#define ADC_ADJUST(x) (x * 3.3f / (1 << 12u) - 1.65f) 
#define ADC_MAX 3.3f
#define ADC_STEP (3.3f/5.f)

uint16_t adc_buffer[SAMPLES];

void init_config_adc();
void sample_mic();
float mic_power();
uint8_t get_intensity(float v);