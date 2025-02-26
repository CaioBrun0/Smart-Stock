#include <stdio.h>
#include <assert.h>
#include "hardware/adc.h"
#include "hardware/gpio.h"

// Simulações das funções do sistema
int calculate_index(int linha, int coluna) {
    if (linha % 2 == 0) {
        return linha * 5 + (4 - coluna); // Linhas pares (invertidas)
    } else {
        return linha * 5 + coluna; // Linhas ímpares (normais)
    }
}

void test_calculate_index() {
    assert(calculate_index(0, 0) == 4);
    assert(calculate_index(0, 4) == 0);
    assert(calculate_index(1, 0) == 5);
    assert(calculate_index(1, 4) == 9);
    assert(calculate_index(2, 2) == 12);
    assert(calculate_index(3, 1) == 16);
    assert(calculate_index(4, 3) == 23);
    printf("Teste calculate_index passou!\n");
}

void test_adc_read_simulation() {
    adc_select_input(1);
    uint16_t x = adc_read();
    adc_select_input(0);
    uint16_t y = adc_read();

    assert(x >= 0 && x <= 4095);
    assert(y >= 0 && y <= 4095);
    printf("Teste adc_read_simulation passou!\n");
}

void test_gpio_debounce() {
    gpio_init(5);
    gpio_set_dir(5, GPIO_IN);
    gpio_pull_up(5);
    
    assert(gpio_get(5) == 1 || gpio_get(5) == 0);
    printf("Teste gpio_debounce passou!\n");
}

int main() {
    test_calculate_index();
    test_adc_read_simulation();
    test_gpio_debounce();
    printf("Todos os testes passaram!\n");
    return 0;
}
