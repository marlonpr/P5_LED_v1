#include "led_panel.h"
#include "driver/ledc.h"
#include "font5x7.h"
#include <math.h>
#include "esp_err.h"


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

#define COLOR_DEPTH 5
#define MAX_LEVEL   ((1 << COLOR_DEPTH) - 1)  // 31



uint8_t smiley[8] = {
    0b00111100,
    0b01000010,
    0b10100101,
    0b10000001,
    0b10100101,
    0b10011001,
    0b01000010,
    0b00111100,
};


static uint8_t gamma5[32];

void init_gamma_table()
{
    const float gamma = 2.2;
    for (int i = 0; i <= MAX_LEVEL; ++i) {
        float normalized = i / (float)MAX_LEVEL;
        float corrected = powf(normalized, gamma);
        gamma5[i] = (uint8_t)(corrected * MAX_LEVEL + 0.5f);
    }
}


//-------------------------------------------//-------------------------------------------


// Choose a high-speed channel/timer so duty updates are as fast as possible
#define OE_SPEED_MODE    LEDC_HIGH_SPEED_MODE
#define OE_TIMER_NUM     LEDC_TIMER_0
#define OE_CHANNEL       LEDC_CHANNEL_0
#define OE_DUTY_RES      LEDC_TIMER_8_BIT    // 256 steps
#define OE_FREQUENCY_HZ  1000000             // 1 MHz PWM freq

void init_oe_pwm(void)
{
    // Configure a high-speed LEDC timer for OE pin
    ledc_timer_config_t timer_conf = {
        .speed_mode       = LEDC_HIGH_SPEED_MODE,
        .duty_resolution  = LEDC_TIMER_6_BIT,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = 1000000,        // 1 MHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer_conf));

    // Map OE pin into that PWM channel
    ledc_channel_config_t chan_conf = {
        .speed_mode     = LEDC_HIGH_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = PIN_OE,
        .duty           = 0,                // start OFF
        .hpoint         = 0,
		.flags = {
            .output_invert = 1   // <-- invert the PWM signal
        }
    };
    ESP_ERROR_CHECK(ledc_channel_config(&chan_conf));
}




// Global brightness in percent (0–100)
static volatile uint8_t global_brightness = 255;

// Call this once after your LEDC timer/channel have been initialized
// to set initial duty according to global_brightness.
static void update_oe_duty(void)
{
    const uint32_t max_duty = (1 << OE_DUTY_RES) - 1;         // 255
    uint32_t duty = (max_duty * global_brightness) / 255;     // scale 0–255
    // Apply it immediately
    ESP_ERROR_CHECK( ledc_set_duty(OE_SPEED_MODE, OE_CHANNEL, duty) );
    ESP_ERROR_CHECK( ledc_update_duty(OE_SPEED_MODE, OE_CHANNEL) );
}

// Call this from wherever you want to change brightness (e.g. CLI, button handler)
void set_global_brightness(uint8_t level)
{
    if (level > 255) {
        level = 255;
    }
    global_brightness = level;
    update_oe_duty();
}


//---------------------------------------------//-------------------------------------------



void draw_bitmap(int x0, int y0, const uint8_t bmp[], int r, int g, int b) {
    for (int y = 0; y < 8; y++) {
        uint8_t row = bmp[y];
        for (int x = 0; x < 8; x++) {
            if (row & (1 << (7 - x))) {
                set_pixel(x0 + x, y0 + y, r, g, b);
            }
        }
    }
}






static uint8_t framebuffer_a[COLOR_DEPTH][PANEL_HEIGHT][PANEL_WIDTH][3];
static uint8_t framebuffer_b[COLOR_DEPTH][PANEL_HEIGHT][PANEL_WIDTH][3];

static volatile uint8_t (*draw_fb)[COLOR_DEPTH][PANEL_HEIGHT][PANEL_WIDTH][3] = &framebuffer_a;
static volatile uint8_t (*disp_fb)[COLOR_DEPTH][PANEL_HEIGHT][PANEL_WIDTH][3] = &framebuffer_b;






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

void scroll_text(const char *str, int y, int r, int g, int b) {
    int len = strlen(str) * 6;
    for (int offset = PANEL_WIDTH; offset > -len-1; offset--) {        
		clear_framebuffer();
        draw_text(str, offset, y, r, g, b);
		swap_buffers();  // Display what was drawn
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}




void init_pins(void)
{
    uint64_t mask =  (1ULL<<PIN_R1) | (1ULL<<PIN_G1) | (1ULL<<PIN_B1) |
                     (1ULL<<PIN_R2) | (1ULL<<PIN_G2) | (1ULL<<PIN_B2) |
                     (1ULL<<PIN_A)  | (1ULL<<PIN_B)  | (1ULL<<PIN_C)  |
                     (1ULL<<PIN_CLK)| (1ULL<<PIN_LAT);

    gpio_config_t io_conf = {
        .pin_bit_mask = mask,
        .mode = GPIO_MODE_OUTPUT,
        .pull_down_en = 0,
        .pull_up_en = 0,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    //gpio_set_level(PIN_OE, 1);
    gpio_set_level(PIN_LAT, 0);
    gpio_set_level(PIN_CLK, 0);
}

void set_pixel(int x, int y, int r, int g, int b)
{
    if (x < 0 || x >= PANEL_WIDTH || y < 0 || y >= PANEL_HEIGHT) return;

    for (int bit = 0; bit < COLOR_DEPTH; bit++) {
        (*draw_fb)[bit][y][x][0] = (r >> bit) & 1;
        (*draw_fb)[bit][y][x][1] = (g >> bit) & 1;
        (*draw_fb)[bit][y][x][2] = (b >> bit) & 1;
    }
}

void swap_buffers()
{
    volatile uint8_t (*temp)[COLOR_DEPTH][PANEL_HEIGHT][PANEL_WIDTH][3] = draw_fb;
    draw_fb = disp_fb;
    disp_fb = temp;
}


void clear_framebuffer()
{
    memset((void*)draw_fb, 0, sizeof(framebuffer_a));
}



void refresh_display_task(void *arg)
{

	//const int base_time_us = 200;  // adjust for overall brightness
	while (true) {
        for (int bit = 0; bit < COLOR_DEPTH; bit++) {			
            for (int row = 0; row < 8; row++) 
		{

                //gpio_set_level(PIN_OE, 1);
				// Duty = max → PWM is always HIGH → OE stays HIGH → panel off
				ledc_set_duty(OE_SPEED_MODE, OE_CHANNEL, 0);
				ledc_update_duty(OE_SPEED_MODE, OE_CHANNEL);

					//esp_rom_delay_us(base_time_us << bit);
				esp_rom_delay_us(5);


                gpio_set_level(PIN_A, row & 0x01);
                gpio_set_level(PIN_B, (row >> 1) & 0x01);
                gpio_set_level(PIN_C, (row >> 2) & 0x01);




                for (int col = 0; col < 128; col++) {
                    int x = col % 64;
                    int group = col >= 64 ? 0 : 1;

                    int y0 = row + (group * 8);
                    int y1 = row + ((group + 2) * 8);

                    int r1 = (*disp_fb)[bit][y0][x][0];
                    int g1 = (*disp_fb)[bit][y0][x][1];
                    int b1 = (*disp_fb)[bit][y0][x][2];

                    int r2 = (*disp_fb)[bit][y1][x][0];
                    int g2 = (*disp_fb)[bit][y1][x][1];
                    int b2 = (*disp_fb)[bit][y1][x][2];

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

				//esp_rom_delay_us(150);

                //gpio_set_level(PIN_OE, 0);
				// Duty = 0 → PWM is always LOW → OE stays LOW → panel on
				//ledc_set_duty(OE_SPEED_MODE, OE_CHANNEL, (1 << OE_DUTY_RES) - 1);
				//ledc_update_duty(OE_SPEED_MODE, OE_CHANNEL);
				update_oe_duty();


				//esp_rom_delay_us(base_time_us << bit);
				esp_rom_delay_us(120);
            }
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
    for (int y = 0; y < PANEL_HEIGHT; y++) {
        for (int x = 0; x < PANEL_WIDTH; x++) {
            int val = ((x / 8 + y / 8) % 2) ? 31 : 0;  // Use 7 (max) or 0 (min) for 3-bit
            set_pixel(x, y, val, val, val);
        }
    }
}

//------------------------------------------------------------------


// Updated gradient: red → green → blue → red
// Masks out only the 'bit' plane from each 0..maxLevel value
static inline void write_pixel_to_draw(int bit,
                                       int y,
                                       int x,
                                       uint8_t r_val,
                                       uint8_t g_val,
                                       uint8_t b_val)
{
    (*draw_fb)[bit][y][x][0] = (r_val >> bit) & 1;
    (*draw_fb)[bit][y][x][1] = (g_val >> bit) & 1;
    (*draw_fb)[bit][y][x][2] = (b_val >> bit) & 1;
}

void hsv_to_rgb_gamma(float h, float s, float v, uint8_t* r, uint8_t* g, uint8_t* b)
{
    float c = v * s;
    float h_prime = fmodf(h / 60.0f, 6);
    float x = c * (1 - fabsf(fmodf(h_prime, 2) - 1));
    float m = v - c;

    float r_f = 0, g_f = 0, b_f = 0;

    if      (h_prime >= 0 && h_prime < 1) { r_f = c; g_f = x; b_f = 0; }
    else if (h_prime >= 1 && h_prime < 2) { r_f = x; g_f = c; b_f = 0; }
    else if (h_prime >= 2 && h_prime < 3) { r_f = 0; g_f = c; b_f = x; }
    else if (h_prime >= 3 && h_prime < 4) { r_f = 0; g_f = x; b_f = c; }
    else if (h_prime >= 4 && h_prime < 5) { r_f = x; g_f = 0; b_f = c; }
    else if (h_prime >= 5 && h_prime < 6) { r_f = c; g_f = 0; b_f = x; }

    int r_raw = (int)((r_f + m) * MAX_LEVEL + 0.5f);
    int g_raw = (int)((g_f + m) * MAX_LEVEL + 0.5f);
    int b_raw = (int)((b_f + m) * MAX_LEVEL + 0.5f);

    // Clamp just in case
    if (r_raw > MAX_LEVEL) r_raw = MAX_LEVEL;
    if (g_raw > MAX_LEVEL) g_raw = MAX_LEVEL;
    if (b_raw > MAX_LEVEL) b_raw = MAX_LEVEL;

    *r = gamma5[r_raw];
    *g = gamma5[g_raw];
    *b = gamma5[b_raw];
}


void test_gradient()
{
    for (int bit = 0; bit < COLOR_DEPTH; ++bit) {
        for (int y = 0; y < PANEL_HEIGHT; ++y) {
            for (int x = 0; x < PANEL_WIDTH; ++x) {
                float hue = (x / (float)(PANEL_WIDTH - 1)) * 360.0f;
                uint8_t r, g, b;
                hsv_to_rgb_gamma(hue, 1.0f, 1.0f, &r, &g, &b);
                write_pixel_to_draw(bit, y, x, r, g, b);
            }
        }
    }
}



//---------------------------------------------------------------------


void test_pixel_by_pixel_fill()
{
    clear_framebuffer();     // Clear draw framebuffer
    swap_buffers();          // Show black screen
    clear_framebuffer();     // Clear the new draw_fb too
    vTaskDelay(pdMS_TO_TICKS(500));

    for (int y = 0; y < PANEL_HEIGHT; y++) {
        for (int x = 0; x < PANEL_WIDTH; x++) {
            set_pixel(x, y, 31, 31, 31);  // Add this pixel to the framebuffer

            // Show this pixel plus all previous ones
            swap_buffers();           // Show accumulated pixels
            vTaskDelay(pdMS_TO_TICKS(10));

            // Copy framebuffer state back to draw_fb (since swap flips them)
            memcpy((void*)draw_fb, (void*)disp_fb, sizeof(framebuffer_a));
        }
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
}

