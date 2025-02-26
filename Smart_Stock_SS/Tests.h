// arquivo.h

#ifndef ARQUIVO_H
#define ARQUIVO_H

#include <stdio.h>
#include <assert.h>
#include "hardware/adc.h"
#include "hardware/gpio.h"

// Simulações das funções do sistema
int calculate_index(int linha, int coluna);

void test_calculate_index();

void test_adc_read_simulation();

void test_gpio_debounce();

#endif // ARQUIVO_H
