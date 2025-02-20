#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "pico/stdlib.h"
#include "pico/stdlib.h"
#include "hardware/adc.h"     
#include "hardware/i2c.h"   
#include "hardware/timer.h"
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
int led_index = 0;
uint8_t red = 0 , green = 255, blue = 0;
float intensity = 1.0;  // Brilho inicial

//Variaveis para tratar os botões
const uint32_t debounce_time_ms = 200;
volatile bool office_hours = true;
volatile bool not_office_hours = false;
absolute_time_t last_interrupt_time = 0;
struct repeating_timer timer;

volatile bool chamou = false;


/* ================================ (inicio) MATRIZ =================================*/

struct pixel_t {
  uint8_t G, R, B;
  float intensity_m;
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t;

npLED_t leds[LED_COUNT];

PIO np_pio;
uint sm;

void npInit(uint pin) {
  uint offset = pio_add_program(pio0, &ws2818b_program);
  np_pio = pio0;
  sm = pio_claim_unused_sm(np_pio, false);
  if (sm < 0) {
      np_pio = pio1;
      sm = pio_claim_unused_sm(np_pio, true);
  }
  ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);
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

void npWrite() {
  for (uint i = 0; i < LED_COUNT; ++i) {
      pio_sm_put_blocking(np_pio, sm, leds[i].G);
      pio_sm_put_blocking(np_pio, sm, leds[i].R);
      pio_sm_put_blocking(np_pio, sm, leds[i].B);
  }
  sleep_us(100);
}

void npSetLEDIntensity(uint index, uint8_t r, uint8_t g, uint8_t b, float intensity) {
  if (intensity < 0) intensity = 0;
  if (intensity > 1) intensity = 1;

  leds[index].R = (uint8_t)(r * intensity);
  leds[index].G = (uint8_t)(g * intensity);
  leds[index].B = (uint8_t)(b * intensity);
  leds[index].intensity_m = intensity;

}


    /* ================================ (FIM) MATRIZ =================================*/


//Função de interrupção
void gpio_irq_handler(uint gpio, uint32_t events) {
  absolute_time_t now = get_absolute_time();
  if (absolute_time_diff_us(last_interrupt_time, now) < debounce_time_ms * 1000)
      return;

  last_interrupt_time = now;
  if (gpio == button_A){
      office_hours = true;
      not_office_hours = false;

  }else if (gpio == button_B){
      office_hours = false;
      not_office_hours = true;
  }
}

bool timer_callback(){
  chamou = true;

  return true;

  
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

    //Matriz
    npInit(PIN_MATRIZ);

}

int main(){
  stdio_init_all();
  initialization();
  srand(time(NULL));

    
  for (int j = 0; j <= 24; j++) { 
    float i = 0.9;  
    float intensity = (float)i / 10.0; // Corrigido para conversão explícita
    npSetLEDIntensity(j, 0, 255, 0, intensity);  
    npWrite();  
    sleep_ms(50);  

  } 

  //sleep_ms(5000);
  gpio_set_irq_enabled_with_callback(button_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
  gpio_set_irq_enabled_with_callback(button_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
  add_repeating_timer_ms(3000, timer_callback, NULL, &timer);

  while (true) {

    if (office_hours){
      
      //func_office_hours();

    } 
    
    else if (not_office_hours){
      //func_not_office_hours();
    }

    if (chamou) {
      printf("Chamou a função de callback\n");
      //Controle de intensidade
    
      int lower_margin = 0;
      int upper_margin = 24;
      int range = upper_margin - lower_margin + 1;
    
      int random_number = rand() % range + lower_margin;
      intensity = leds[random_number].intensity_m;
      int is_red = leds[random_number].R;
      int is_green = leds[random_number].G;
      
      printf("Numero escolhido: %d\n", random_number);
      printf("Itensidade do LED: %.2f\n", intensity);
      float new_intensity = intensity * 0.9;
      printf("Nova intensidade: %.2f\n", new_intensity);

      if(is_red > 0){
        printf("é vermelho: %d\n", is_red);
        sleep_ms(1000);
      }

      //Sensor de presença
      if (new_intensity <= 0.1 && is_red == 0){
        npSetLEDIntensity(random_number, 255, 0, 0, 0.1);  
        printf("Itensidade muito baixa, fica vermelho\n");

      } else if (is_green > 0){
        npSetLEDIntensity(random_number, 0, 255, 0, new_intensity);
        printf("Tirando 0.01 do LED: %d\n", random_number);  
      }
      printf("Saiu do callback\n");
      printf("\n");
      npWrite();
      chamou = false;

      
    }
    sleep_ms(200);
    
  }

}


void func_office_hours(){

}

void func_not_office_hours(){

}
