#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "inc/ssd1306.h"
#include "hardware/i2c.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"
#include "ws2818b.pio.h"

// Display
const uint I2C_SDA = 14;
const uint I2C_SCL = 15;

// LED RGB (Pedestre)
#define LED_R_PIN 13
#define LED_G_PIN 11
#define LED_B_PIN 12

// Definição do número de LEDs e pino - matriz (semaforo)
#define LED_COUNT 25
#define LED_PIN 7

// Definição de pixel GRB
struct pixel_t
{
    uint8_t G, R, B; // Três valores de 8-bits compõem um pixel.
};
typedef struct pixel_t pixel_t;
typedef pixel_t npLED_t; // Mudança de nome de "struct pixel_t" para "npLED_t" por clareza.

// Declaração do buffer de pixels que formam a matriz.
npLED_t leds[LED_COUNT];

// Variáveis para uso da máquina PIO.
PIO np_pio;
uint sm;

// Botão
#define BTN_A_PIN 5

int A_state = 0; // Botão A está pressionado?

/**
 * Inicializa a máquina PIO para controle da matriz de LEDs.
 */
void npInit(uint pin)
{
    // Cria programa PIO.
    uint offset = pio_add_program(pio0, &ws2818b_program);
    np_pio = pio0;

    // Toma posse de uma máquina PIO.
    sm = pio_claim_unused_sm(np_pio, false);
    if (sm < 0)
    {
        np_pio = pio1;
        sm = pio_claim_unused_sm(np_pio, true); // Se nenhuma máquina estiver livre, panic!
    }

    // Inicia programa na máquina PIO obtida.
    ws2818b_program_init(np_pio, sm, offset, pin, 800000.f);

    // Limpa buffer de pixels.
    for (uint i = 0; i < LED_COUNT; ++i)
    {
        leds[i].R = 0;
        leds[i].G = 0;
        leds[i].B = 0;
    }
}

/**
 * Atribui uma cor RGB a um LED.
 */
void npSetLED(const uint index, const uint8_t r, const uint8_t g, const uint8_t b)
{
    leds[index].R = r;
    leds[index].G = g;
    leds[index].B = b;
}

/**
 * Limpa o buffer de pixels.
 */
void npClear()
{
    for (uint i = 0; i < LED_COUNT; ++i)
        npSetLED(i, 0, 0, 0);
}

/**
 * Escreve os dados do buffer nos LEDs.
 */
void npWrite()
{
    // Escreve cada dado de 8-bits dos pixels em sequência no buffer da máquina PIO.
    for (uint i = 0; i < LED_COUNT; ++i)
    {
        pio_sm_put_blocking(np_pio, sm, leds[i].G);
        pio_sm_put_blocking(np_pio, sm, leds[i].R);
        pio_sm_put_blocking(np_pio, sm, leds[i].B);
    }
    sleep_us(100); // Espera 100us, sinal de RESET do datasheet.
}

// Função para exibir texto no OLED
void display_text(const char *text[], int num_lines)
{
    // Preparar área de renderização para o display (ssd1306_width pixels por ssd1306_n_pages páginas)
    struct render_area frame_area = {
        start_column : 0,
        end_column : ssd1306_width - 1,
        start_page : 0,
        end_page : ssd1306_n_pages - 1
    };

    calculate_render_area_buffer_length(&frame_area);

    // Zera o display inteiro
    uint8_t ssd[ssd1306_buffer_length];
    memset(ssd, 0, ssd1306_buffer_length);

    int y = 0;
    for (int i = 0; i < num_lines; i++)
    {
        char buffer[22];
        snprintf(buffer, sizeof(buffer), "%.21s", text[i]);
        ssd1306_draw_string(ssd, 5, y, buffer);
        y += 8;
    }
    render_on_display(ssd, &frame_area);
}

// Funções para controlar os semáforos
void SinalAbertoPedestre()
{
    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);

    const char *text[] = {
        "SINAL ABERTO - ",
        "ATRAVESSAR COM",
        "CUIDADO"};
    display_text(text, 3);
}

void SinalAtencaoPedestre()
{
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 1);
    gpio_put(LED_B_PIN, 0);

    const char *text[] = {
        "SINAL DE ",
        "ATEN\xC7\xC3O - ",
        "APRESSE-SE"};
    display_text(text, 3);
}

void SinalFechadoPedestre()
{
    gpio_put(LED_R_PIN, 1);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);

    const char *text[] = {
        "SINAL FECHADO - ",
        "AGUARDE"};
    display_text(text, 2);
}

void SinalAbertoCarros()
{
    npSetLED(20, 0, 0, 0);   // desliga vermelho
    npSetLED(24, 0, 255, 0); // Verde para carros
    npWrite();
}

void SinalAtencaoCarros()
{
    npSetLED(20, 0, 0, 0);     // desliga vermelho (importante quando muda de ciclo)
    npSetLED(24, 0, 0, 0);     // desliga verde
    npSetLED(22, 255, 255, 0); // Amarelo para carros
    npWrite();
}

void SinalFechadoCarros()
{
    npSetLED(22, 0, 0, 0);   // desliga amarelo
    npSetLED(20, 255, 0, 0); // Vermelho para carros
    npWrite();
}

int WaitWithRead(int timeMS)
{
    for (int i = 0; i < timeMS; i += 100)
    {
        A_state = !gpio_get(BTN_A_PIN);
        if (A_state == 1)
        {
            return 1;
        }
        sleep_ms(100);
    }
    return 0;
}

void cicloNormal()
{
    // Semáforo de carros aberto, pedestres fechado
    SinalAbertoCarros();
    SinalFechadoPedestre();
    if (WaitWithRead(8000))
        return;

    // Semáforo de carros amarelo, pedestres fechado
    SinalAtencaoCarros();
    if (WaitWithRead(2000))
        return;

    // Semáforo de carros fechado, pedestres aberto
    SinalFechadoCarros();
    SinalAbertoPedestre();
    if (WaitWithRead(8000))
        return;

    // Semáforo de carros fechado, pedestres amarelo
    SinalFechadoCarros();
    SinalAtencaoPedestre();
    if (WaitWithRead(2000))
        return;
}

void cicloInterrompido()
{
    // Semáforo de carros amarelo, pedestres fechado
    SinalAtencaoCarros();
    SinalFechadoPedestre();
    sleep_ms(5000);

    // Semáforo de carros fechado, pedestres aberto
    SinalFechadoCarros();
    SinalAbertoPedestre();
    sleep_ms(8000);

    // Semáforo de carros fechado, pedestres amarelo
    SinalFechadoCarros();
    SinalAtencaoPedestre();
    sleep_ms(2000);
}

int main()
{
    stdio_init_all();

    // INICIANDO LEDS
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    // Inicializa matriz de LEDs NeoPixel.
    npInit(LED_PIN);
    npClear();

    // INICIANDO BOTÃO
    gpio_init(BTN_A_PIN);
    gpio_set_dir(BTN_A_PIN, GPIO_IN);
    gpio_pull_up(BTN_A_PIN);

    // Inicialização do i2c e OLED
    i2c_init(i2c1, ssd1306_i2c_clock * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);
    ssd1306_init();

    while (true)
    {
        // Verifica se o botão foi pressionado
        A_state = !gpio_get(BTN_A_PIN);

        if (A_state)
        { // Botão foi pressionado, executa o ciclo interrompido uma vez
            cicloInterrompido();
            A_state = 0; // Reseta o estado do botão após executar o ciclo interrompido
        }
        else
        {
            cicloNormal();
        }
    }

    return 0;
}