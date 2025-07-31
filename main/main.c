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

    vTaskDelay(pdMS_TO_TICKS(3000));

    while (1) {



		clear_framebuffer();
		test_solid_color(0,7,7);
		swap_buffers();  // Display what was drawn
		vTaskDelay(pdMS_TO_TICKS(3000));

 
        clear_framebuffer();
		test_checkerboard();
		swap_buffers();  // Display what was drawn
        vTaskDelay(pdMS_TO_TICKS(3000));

        clear_framebuffer();
		test_gradient();
		swap_buffers();  // Display what was drawn
        vTaskDelay(pdMS_TO_TICKS(3000));

        clear_framebuffer();
		test_pixel_by_pixel_fill();  // Final test
		//swap_buffers();  // Display what was drawn
        vTaskDelay(pdMS_TO_TICKS(3000));

		
		clear_framebuffer();
		draw_text("HELLO", 4, 2, 7, 7, 7);
		swap_buffers();  // Display what was drawn
		vTaskDelay(pdMS_TO_TICKS(1000));

		clear_framebuffer();
		scroll_text("ESP32 LED DEMO", 10, 7, 0, 0);
		swap_buffers();  // Display what was drawn
    	vTaskDelay(pdMS_TO_TICKS(500));

		clear_framebuffer();
		draw_bitmap(28, 12, smiley, 0, 7, 0);
		swap_buffers();  // Display what was drawn
		vTaskDelay(pdMS_TO_TICKS(1000));



    }
}

/*
Max 3-bit: R,G,B values 0–7

Yellow: R=7, G=7, B=0

Cyan: R=0, G=7, B=7

Magenta: R=7, G=0, B=7

		clear_framebuffer();
	    test_solid_color(7, 0, 0);  // 
		swap_buffers();  // Display what was drawn
		vTaskDelay(pdMS_TO_TICKS(1000));
 
        test_checkerboard();
        vTaskDelay(pdMS_TO_TICKS(1000));

        test_gradient();
        vTaskDelay(pdMS_TO_TICKS(1000));

        test_pixel_by_pixel_fill();  // Final test
        vTaskDelay(pdMS_TO_TICKS(2000));

		
		draw_text("HELLO", 4, 2, 1, 1, 1);
		vTaskDelay(pdMS_TO_TICKS(1000));

		scroll_text("ESP32 LED DEMO", 10, 7, 0, 0);
    	vTaskDelay(pdMS_TO_TICKS(500));

		draw_bitmap(28, 12, smile, 0, 7, 0);
		vTaskDelay(pdMS_TO_TICKS(1000));









Gray (various shades):

Dark Gray: R=1, G=1, B=1

Mid-tone Gray: R=3, G=3, B=3

Light Gray: R=6, G=6, B=6

Brown: R=4, G=2, B=0

Purple: R=4, G=0, B=4

Olive: R=4, G=4, B=0

Navy Blue: R=0, G=0, B=4

Teal: R=0, G=4, B=4

Pink: R=7, G=4, B=4







		clear_framebuffer();
		test_solid_color(0,7,7);
		swap_buffers();  // Display what was drawn
		vTaskDelay(pdMS_TO_TICKS(1000));

 
        clear_framebuffer();
		test_checkerboard();
		swap_buffers();  // Display what was drawn
        vTaskDelay(pdMS_TO_TICKS(1000));

        clear_framebuffer();
		test_gradient();
		swap_buffers();  // Display what was drawn
        vTaskDelay(pdMS_TO_TICKS(1000));

        clear_framebuffer();
		test_pixel_by_pixel_fill();  // Final test
		swap_buffers();  // Display what was drawn
        vTaskDelay(pdMS_TO_TICKS(2000));

		
		clear_framebuffer();
		draw_text("HELLO", 4, 2, 7, 7, 7);
		swap_buffers();  // Display what was drawn
		vTaskDelay(pdMS_TO_TICKS(1000));

		clear_framebuffer();
		scroll_text("ESP32 LED DEMO", 10, 7, 0, 0);
		swap_buffers();  // Display what was drawn
    	vTaskDelay(pdMS_TO_TICKS(500));

		clear_framebuffer();
		draw_bitmap(28, 12, smiley, 0, 7, 0);
		swap_buffers();  // Display what was drawn
		vTaskDelay(pdMS_TO_TICKS(1000));







	*/
