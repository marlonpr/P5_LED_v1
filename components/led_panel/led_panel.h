#ifndef LED_PANEL_H
#define LED_PANEL_H

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include <string.h>
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

void draw_text(const char *str, int x, int y, int r, int g, int b);
void clear_panel();
void scroll_text(const char *str, int y, int r, int g, int b);


#endif // LED_PANEL_H