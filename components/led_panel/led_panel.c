#include "led_panel.h"

#include "font5x7.h"

void draw_char(char c, int x, int y, int r, int g, int b) {
    if (c < 32 || c > 126) return;
    const uint8_t *glyph = font5x7[c - 32];
    for (int col = 0; col < 5; col++) {
        uint8_t column = glyph[col];
        for (int row = 0; row < 7; row++) {
            if (column & (1 << row)) {
                set_pixel(x + col, y + row, r, g, b);
            }
        }
    }
}


void draw_text(const char *str, int x, int y, int r, int g, int b) {
    while (*str) {
        draw_char(*str, x, y, r, g, b);
        x += 6;  // 5px + 1 space
        str++;
    }
}

void clear_panel() {
    for (int y = 0; y < PANEL_HEIGHT; y++) {
        for (int x = 0; x < PANEL_WIDTH; x++) {
            set_pixel(x, y, 0, 0, 0);
        }
    }
}


void scroll_text(const char *str, int y, int r, int g, int b) {
    int len = strlen(str) * 6;
    for (int offset = PANEL_WIDTH; offset > -len; offset--) {
        clear_panel();
		//clear_framebuffer();
        draw_text(str, offset, y, r, g, b);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}



#define PIN_R1   GPIO_NUM_2
#define PIN_G1   GPIO_NUM_4
#define PIN_B1   GPIO_NUM_5
#define PIN_R2   GPIO_NUM_18
#define PIN_G2   GPIO_NUM_19
#define PIN_B2   GPIO_NUM_21
#define PIN_CLK  GPIO_NUM_13
#define PIN_LAT  GPIO_NUM_12
#define PIN_OE   GPIO_NUM_14
#define PIN_A    GPIO_NUM_15
#define PIN_B    GPIO_NUM_22
#define PIN_C    GPIO_NUM_23

#define PANEL_WIDTH   64
#define PANEL_HEIGHT  32

static const char *TAG = "LED_PANEL";

static uint8_t framebuffer[PANEL_HEIGHT][PANEL_WIDTH][3];  // RGB

void init_pins(void)
{
    uint64_t mask =  (1ULL<<PIN_R1) | (1ULL<<PIN_G1) | (1ULL<<PIN_B1) |
                     (1ULL<<PIN_R2) | (1ULL<<PIN_G2) | (1ULL<<PIN_B2) |
                     (1ULL<<PIN_A)  | (1ULL<<PIN_B)  | (1ULL<<PIN_C)  |
                     (1ULL<<PIN_CLK)| (1ULL<<PIN_LAT)| (1ULL<<PIN_OE);

    gpio_config_t io_conf = {
        .pin_bit_mask = mask,
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = 0,
        .pull_up_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    gpio_set_level(PIN_OE, 1);
    gpio_set_level(PIN_LAT, 0);
    gpio_set_level(PIN_CLK, 0);
}

void set_pixel(int x, int y, int r, int g, int b)
{
    if (x >= 0 && x < PANEL_WIDTH && y >= 0 && y < PANEL_HEIGHT) {
        framebuffer[y][x][0] = r;
        framebuffer[y][x][1] = g;
        framebuffer[y][x][2] = b;
    }
}

void clear_framebuffer()
{
    memset(framebuffer, 0, sizeof(framebuffer));
}

void refresh_display_task(void *arg)
{
    while (true) {
        for (int row = 0; row < 8; row++) {
            gpio_set_level(PIN_A, row & 0x01);
            gpio_set_level(PIN_B, (row >> 1) & 0x01);
            gpio_set_level(PIN_C, (row >> 2) & 0x01);

            gpio_set_level(PIN_OE, 1);

            for (int col = 0; col < 128; col++) {
                int x = col % 64;
                int group = col >= 64 ? 0 : 1;

                int y0 = row + (group * 8);
                int y1 = row + ((group + 2) * 8);

                int r1 = framebuffer[y0][x][0];
                int g1 = framebuffer[y0][x][1];
                int b1 = framebuffer[y0][x][2];

                int r2 = framebuffer[y1][x][0];
                int g2 = framebuffer[y1][x][1];
                int b2 = framebuffer[y1][x][2];

                gpio_set_level(PIN_R1, r1);
                gpio_set_level(PIN_G1, g1);
                gpio_set_level(PIN_B1, b1);

                gpio_set_level(PIN_R2, r2);
                gpio_set_level(PIN_G2, g2);
                gpio_set_level(PIN_B2, b2);

                gpio_set_level(PIN_CLK, 1);
                gpio_set_level(PIN_CLK, 0);
            }

            gpio_set_level(PIN_LAT, 1);
            gpio_set_level(PIN_LAT, 0);

            gpio_set_level(PIN_OE, 0);
            esp_rom_delay_us(100);  // Adjust brightness
        }
    }
}

void test_solid_color(int r, int g, int b)
{
    for (int y = 0; y < PANEL_HEIGHT; y++)
        for (int x = 0; x < PANEL_WIDTH; x++)
            set_pixel(x, y, r, g, b);
}

void test_checkerboard()
{
    for (int y = 0; y < PANEL_HEIGHT; y++)
        for (int x = 0; x < PANEL_WIDTH; x++) {
            int val = ((x / 8 + y / 8) % 2);
            set_pixel(x, y, val, val, val);
        }
}

void test_gradient()
{
    for (int y = 0; y < PANEL_HEIGHT; y++)
        for (int x = 0; x < PANEL_WIDTH; x++) {
            set_pixel(x, y, x % 2, y % 2, (x + y) % 2);
        }
}

void test_pixel_by_pixel_fill()
{
    clear_framebuffer();
    for (int y = 0; y < PANEL_HEIGHT; y++) {
        for (int x = 0; x < PANEL_WIDTH; x++) {
            set_pixel(x, y, 1, 1, 1);  // White
            vTaskDelay(pdMS_TO_TICKS(10));
        }
    }
}
