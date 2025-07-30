#include "led_panel.h"

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


void app_main(void)
{
    init_pins();
    xTaskCreate(refresh_display_task, "refresh_display_task", 4096, NULL, 1, NULL);

    vTaskDelay(pdMS_TO_TICKS(1000));

    while (1) {
	/*
	    test_solid_color(1, 0, 0);  // Red
        vTaskDelay(pdMS_TO_TICKS(1000));

        test_solid_color(0, 1, 0);  // Green
        vTaskDelay(pdMS_TO_TICKS(1000));

        test_solid_color(0, 0, 1);  // Blue
        vTaskDelay(pdMS_TO_TICKS(1000));

        test_checkerboard();
        vTaskDelay(pdMS_TO_TICKS(1000));

        test_gradient();
        vTaskDelay(pdMS_TO_TICKS(1000));

        test_pixel_by_pixel_fill();  // Final test
        vTaskDelay(pdMS_TO_TICKS(2000));
	*/

		//clear_framebuffer();
		draw_text("HELLO", 4, 2, 1, 1, 1);
		vTaskDelay(pdMS_TO_TICKS(1000));

		scroll_text("ESP32 LED DEMO", 10, 1, 0, 0);
    	vTaskDelay(pdMS_TO_TICKS(500));

   		clear_panel();
		draw_bitmap(28, 12, smiley, 0, 1, 0);
		vTaskDelay(pdMS_TO_TICKS(1000));

    }
}
