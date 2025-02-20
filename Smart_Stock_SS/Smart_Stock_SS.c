#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"     
#include "hardware/pwm.h"   
#include "hardware/i2c.h"   
#include "ws2818b.pio.h"
#include "lib/ssd1306.h"   
#include "lib/font.h"

//Pins
#define button_A 5
#define button_B 6
#define buzzer_A 21
#define buzzer_B 10
#define VRX 26
#define VRY 27
#define LED_r 13
#define LED_g 11
#define LED_b 12

//Display
#define I2C_PORT i2c1 
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3c
uint pwm_wrap = 4096;

ssd1306_t ssd;
#define WIDTH 128 
#define HEIGHT 64 
#define SQUARE_SIZE 8

//Matriz
#define DIGIT_SIZE 5
#define LED_COUNT 25
#define PIN_MATRIZ 7

//Variaveis para tratar os botões
const uint32_t debounce_time_ms = 200;
volatile bool button_a_pressed = false;
volatile bool button_b_pressed = false;
absolute_time_t last_interrupt_time = 0;

// Definição de pixel GRB
struct pixel_t {
    uint8_t G, R, B; // Três valores de 8-bits compõem um pixel.
  };
  typedef struct pixel_t pixel_t;
  typedef pixel_t npLED_t; // Mudança de nome de "struct pixel_t" para "npLED_t" por clareza.
  
  // Declaração do buffer de pixels que formam a matriz.
  npLED_t leds[LED_COUNT];
  
  // Variáveis para uso da máquina PIO.
  PIO np_pio;
  uint sm;
  
  /**
   * Inicializa a máquina PIO para controle da matriz de LEDs.
   */
  void npInit(uint pin) {
  
    // Cria programa PIO.
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;
  
    // Toma posse de uma máquina PIO.
    sm = pio_claim_unused_sm(np_pio, false);
    if (sm < 0) {
      np_pio = pio1;
      sm = pio_claim_unused_sm(np_pio, true); // Se nenhuma máquina estiver livre, panic!
    }
  
    // Inicia programa na máquina PIO obtida.
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);
  
    // Limpa buffer de pixels.
    for (uint i = 0; i < LED_COUNT; ++i) {
      leds[i].R = 0;
      leds[i].G = 0;
      leds[i].B = 0;
    }
  }
  
  
  void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b) {
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
  }
  
  //Limpa o buffer
  void npClear() {
    for (uint i = 0; i < LED_COUNT; ++i)
      npSetLED(i, 0, 0, 0);
  }
  
  //Escreve os dados no buffer
  void npWrite() {
    // Escreve cada dado de 8-bits dos pixels em sequência no buffer da máquina PIO.
    for (uint i = 0; i < LED_COUNT; ++i) {
      pio_sm_put_blocking(np_pio, sm, leds[i].G);
      pio_sm_put_blocking(np_pio, sm, leds[i].R);
      pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
    sleep_us(100); // Espera 100us, sinal de RESET do datasheet.
  }
  
  // Função para converter a posição do matriz para uma posição do vetor.
  int getIndex(int x, int y) {
      if (y % 2 == 0) {
          return 24-(y * 5 + x); 
      } else {
          return 24-(y * 5 + (4 - x)); 
      }
  }
  
  // Atualizar os LEDs com os valores ajustados
  void brightness(int matriz[5][5][3]){
  
      // Atualizar os LEDs com os valores ajustados
      for (int linha = 0; linha < 5; linha++) {
          for (int coluna = 0; coluna < 5; coluna++) {
              int posicao = getIndex(linha, coluna);
              npSetLED(posicao, matriz[coluna][linha][0], matriz[coluna][linha][1], matriz[coluna][linha][2]);
          }
      }
  
      npWrite();
  
    }  

//Inicializa os pinos
void initialization(){
    //Joystick
    stdio_init_all();
    adc_init();
    adc_gpio_init(26);
    adc_gpio_init(27);

    //LED RGB
    gpio_init(LED_r);
    gpio_set_dir(LED_r, GPIO_OUT);
    gpio_put(LED_r, 0);

    gpio_init(LED_g);
    gpio_set_dir(LED_g, GPIO_OUT);
    gpio_put(LED_g, 0);
    
    gpio_init(LED_b);
    gpio_set_dir(LED_b, GPIO_OUT);
    gpio_put(LED_b, 0);

    //Buzzers
    gpio_init(buzzer_A);
    gpio_set_dir(buzzer_A, GPIO_OUT);
    gpio_put(buzzer_A, 0); 

    gpio_init(buzzer_B);
    gpio_set_dir(buzzer_B, GPIO_OUT);
    gpio_put(buzzer_B, 0); 

    //Botões
    gpio_init(button_A);
    gpio_set_dir(button_A, GPIO_IN);
    gpio_pull_up(button_A);

    gpio_init(button_B);
    gpio_set_dir(button_B, GPIO_IN);
    gpio_pull_up(button_B);


    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA); // Pull up the data line
    gpio_pull_up(I2C_SCL); // Pull up the clock line
    
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display

}

int main()
{
    stdio_init_all();
    initialization();

    while (true) {

        sleep_ms(1000);
    }
}
