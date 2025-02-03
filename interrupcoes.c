#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "interrupcoes.pio.h"

#define BOTAO_A 5
#define BOTAO_B 6
#define LED_RED 13 
#define LED_GREEN 11 
#define LED_BLUE 12 
#define CONTADOR_LED 25
#define MATRIZ_LED 7

static volatile uint32_t last_time = 0;
static volatile int number = 0; // Variável para armazenar o número atual

uint32_t ultimo_botao_a = 0;
uint32_t ultimo_botao_b = 0;

#define DEBOUNCE_DELAY 200

struct pixel_t {
    uint8_t G, R, B;
};
typedef struct pixel_t LED_da_matriz;

// Declaração do buffer de pixels que formam a matriz.
LED_da_matriz leds[CONTADOR_LED];

// Variáveis para uso da máquina PIO.
PIO maquina_pio;
uint variavel_maquina_de_estado;

// Protótipos
void inicializacao_maquina_pio(uint pino);
void atribuir_cor_ao_led(const uint indice, const uint8_t r, const uint8_t g, const uint8_t b);
void limpar_o_buffer(void);
void escrever_no_buffer(void);
static void gpio_irq_handler(uint gpio, uint32_t events);
void display_number(int number);

// Inicializa a máquina PIO para controle da matriz de LEDs.
void inicializacao_maquina_pio(uint pino) {
    uint programa_pio, i;
    programa_pio = pio_add_program(pio0, &interrupcoes_program);
    maquina_pio = pio0;

    variavel_maquina_de_estado = pio_claim_unused_sm(maquina_pio, false);
    if (variavel_maquina_de_estado < 0) {
        maquina_pio = pio1;
        variavel_maquina_de_estado = pio_claim_unused_sm(maquina_pio, true);
    }

    interrupcoes_program_init(maquina_pio, variavel_maquina_de_estado, programa_pio, pino, 800000.f);

    for (i = 0; i < CONTADOR_LED; ++i) {
        leds[i].R = 0;
        leds[i].G = 0;
        leds[i].B = 0;
    }
}

// Atribui uma cor RGB a um LED.
void atribuir_cor_ao_led(const uint indice, const uint8_t r, const uint8_t g, const uint8_t b) {
    leds[indice].R = r;
    leds[indice].G = g;
    leds[indice].B = b;
}

// Limpa o buffer de pixels.
void limpar_o_buffer(void) {
    for (uint i = 0; i < CONTADOR_LED; ++i)
        atribuir_cor_ao_led(i, 0, 0, 0);
}

// Escreve os dados do buffer nos LEDs.
void escrever_no_buffer(void) {
    for (uint i = 0; i < CONTADOR_LED; ++i) {
        pio_sm_put_blocking(maquina_pio, variavel_maquina_de_estado, leds[i].G);
        pio_sm_put_blocking(maquina_pio, variavel_maquina_de_estado, leds[i].R);
        pio_sm_put_blocking(maquina_pio, variavel_maquina_de_estado, leds[i].B);
    }
    sleep_us(100);
}

// Função para exibir um número na matriz de LEDs
void display_number(int number) {
    limpar_o_buffer();

    const uint8_t numbers[10][25] = {
        {0, 1, 1, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 1, 1, 0}, // 0

        {0, 1, 0, 0, 0,
         0, 0, 0, 1, 0,
         0, 1, 0, 0, 0,
         0, 0, 0, 1, 0,
         0, 1, 0, 0, 0}, // 1

        {0, 1, 1, 1, 0,
         0, 1, 0, 0, 0,
         0, 1, 1, 1, 0,
         0, 0, 0, 1, 0,
         0, 1, 1, 1, 0}, // 2

        {0, 1, 1, 1, 0,
         0, 0, 0, 1, 0,
         0, 1, 1, 1, 0,
         0, 0, 0, 1, 0,
         0, 1, 1, 1, 0}, // 3

        {0, 1, 0, 0, 0,
         0, 0, 0, 1, 0,
         0, 1, 1, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 0, 1, 0}, // 4

        {0, 1, 1, 1, 0,
         0, 0, 0, 1, 0,
         0, 1, 1, 1, 0,
         0, 1, 0, 0, 0,
         0, 1, 1, 1, 0}, // 5

        {0, 1, 1, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 1, 1, 0,
         0, 1, 0, 0, 0,
         0, 1, 1, 1, 0}, // 6

        {0, 1, 0, 0, 0,
         0, 0, 0, 1, 0,
         0, 1, 0, 0, 0,
         0, 0, 0, 1, 0,
         0, 1, 1, 1, 0}, // 7

        {0, 1, 1, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 1, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 1, 1, 0}, // 8

        {0, 1, 1, 1, 0,
         0, 0, 0, 1, 0,
         0, 1, 1, 1, 0,
         0, 1, 0, 1, 0,
         0, 1, 1, 1, 0}  // 9
    };

    for (int i = 0; i < 25; i++) {
        if (numbers[number][i]) {
            atribuir_cor_ao_led(i, 255, 0, 0); // Vermelho
        } else {
            atribuir_cor_ao_led(i, 0, 0, 0); // Desligado
        }
    }

    escrever_no_buffer();
}
void gpio_irq_handler(uint gpio, uint32_t eventos) {
    uint32_t tempo_atual = to_ms_since_boot(get_absolute_time());

    // Verifica qual botão foi pressionado e atualiza o número exibido
    // Botão B incrementa o número exibido
    if (gpio == BOTAO_B && (tempo_atual - ultimo_botao_b > DEBOUNCE_DELAY)) {
        number = (number + 1) % 10; // Incrementa (mantém entre 0 e 9)
        ultimo_botao_b = tempo_atual;
        printf("Botão B - Número exibido: %d\n", number);
    }
    // Botão A decrementa o número exibido
    else if (gpio == BOTAO_A && (tempo_atual - ultimo_botao_a > DEBOUNCE_DELAY)) {
        number = (number + 9) % 10; // Decrementa (mantém entre 0 e 9)
        ultimo_botao_a = tempo_atual;
        printf("Botão A - Número exibido: %d\n", number);
    }
}

int main(void) {
    stdio_init_all(); // Inicializa a comunicação serial

    gpio_init(LED_RED);
    gpio_set_dir(LED_RED, GPIO_OUT);
    gpio_init(LED_BLUE);
    gpio_set_dir(LED_GREEN, GPIO_OUT);

    gpio_init(BOTAO_A);
    gpio_set_dir(BOTAO_A, GPIO_IN);
    gpio_pull_up(BOTAO_A);

    gpio_init(BOTAO_B);
    gpio_set_dir(BOTAO_B, GPIO_IN);
    gpio_pull_up(BOTAO_B);

    inicializacao_maquina_pio(MATRIZ_LED);

     gpio_init(BOTAO_A); // Botão A
    gpio_set_dir(BOTAO_A, GPIO_IN); // Define como entrada
    gpio_pull_up(BOTAO_A); // Habilita resistor de pull-up
    gpio_set_irq_enabled(BOTAO_A, GPIO_IRQ_EDGE_FALL, true); // Habilita interrupção de borda de descida

    gpio_init(BOTAO_B); // Botão B
    gpio_set_dir(BOTAO_B, GPIO_IN); // Define como entrada
    gpio_pull_up(BOTAO_B); // Habilita resistor de pull-up
    gpio_set_irq_enabled(BOTAO_B, GPIO_IRQ_EDGE_FALL, true); // Habilita interrupção de borda de descida
 
    gpio_set_irq_enabled_with_callback(BOTAO_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
    gpio_set_irq_enabled_with_callback(BOTAO_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);

    bool estado_led_rgb = false;
    display_number(number); 

     uint32_t troca_led_rgb = 0;
    while (true) {
        display_number(number);
     uint32_t tempo_atual = to_ms_since_boot(get_absolute_time()); 
        if (tempo_atual - troca_led_rgb >= 100) {
            estado_led_rgb = !estado_led_rgb; 
            gpio_put(LED_RED, estado_led_rgb); 
            gpio_put(LED_GREEN, 0); 
            gpio_put(LED_BLUE, 0); 
            troca_led_rgb = tempo_atual;
        }

        sleep_ms(10);
    }
}