#ifndef LED_PANEL_H
#define LED_PANEL_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>
#include "inttypes.h"
// Panel dimensions
#define PANEL_WIDTH   64
#define PANEL_HEIGHT  32


// === API Functions ===
void init_pins(void);
void set_pixel(int x, int y, int r, int g, int b);
void clear_framebuffer();
void refresh_display_task(void *arg);
void test_solid_color(int r, int g, int b);
void test_checkerboard();
void test_gradient();
void test_pixel_by_pixel_fill();


void swap_buffers();


void draw_text(const char *str, int x, int y, int r, int g, int b);
void scroll_text(const char *str, int y, int r, int g, int b);


void init_oe_pwm(void);

void init_gamma_table();

void set_global_brightness(uint8_t percent);

void draw_bitmap(int x0, int y0, const uint8_t bmp[], int r, int g, int b);


extern uint8_t smiley[];



#endif // LED_PANEL_H