#ifndef MATRIZ_H
#define MATRIZ_H

#include "ws2812b_animation.h"
#include "pico/stdlib.h"

#define LEDS_VERDES 13
#define LEDS_AMARELOS 7
#define LEDS_VERMELHOS 5
#define MIN_DB_MAPA 35.0f
#define MAX_DB_MAPA 85.0f

#define CANAL_PIO pio0
#define PINO_MATRIZ 7
#define QTD_LEDS 25

void inicializar_matriz();
void atualizar_ledbar(int qtd_leds);
void renderizar();
void limpar_matriz();

#endif