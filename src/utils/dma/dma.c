#include "dma.h"

void config_dma()
{
    dma_cfg = dma_channel_get_default_config(dma_channel);

    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&dma_cfg, false);
    channel_config_set_write_increment(&dma_cfg, true);

    channel_config_set_dreq(&dma_cfg, DREQ_ADC);
}