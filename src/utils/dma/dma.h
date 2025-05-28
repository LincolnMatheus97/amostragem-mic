#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/dma.h"
#include "hardware/adc.h"

uint dma_channel;
dma_channel_config dma_cfg;

void init_config_dma();