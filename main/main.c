#include "led_panel.h"

void drawing_task(void *arg)
{
	 while (1) {

		clear_framebuffer();
		test_solid_color(31,0,31);
		swap_buffers();  // Display what was drawn
		vTaskDelay(pdMS_TO_TICKS(3000));

 
        clear_framebuffer();
		test_checkerboard();
		swap_buffers();  // Display what was drawn
        vTaskDelay(pdMS_TO_TICKS(2000));

        clear_framebuffer();
		test_gradient();
		swap_buffers();  // Display what was drawn
        vTaskDelay(pdMS_TO_TICKS(6000));

        clear_framebuffer();
		test_pixel_by_pixel_fill();  // Final test
		//swap_buffers();  // Display what was drawn
        vTaskDelay(pdMS_TO_TICKS(1000));

		
		clear_framebuffer();
		draw_text("HELLO", 4, 2, 31, 31, 31);
		swap_buffers();  // Display what was drawn
		vTaskDelay(pdMS_TO_TICKS(2000));

		clear_framebuffer();
		draw_text("WORLD", 4, 2, 31, 0, 31);
		swap_buffers();  // Display what was drawn
		vTaskDelay(pdMS_TO_TICKS(2000));

		clear_framebuffer();
		scroll_text("ESP32 LED DEMO", 10, 31, 0, 0);
		swap_buffers();  // Display what was drawn
    	vTaskDelay(pdMS_TO_TICKS(500));

		clear_framebuffer();
		draw_bitmap(28, 12, smiley, 0, 31, 0);
		swap_buffers();  // Display what was drawn
		vTaskDelay(pdMS_TO_TICKS(2000));
		
		

    }
}




void app_main(void)
{
    init_gamma_table();	
	init_pins();
	init_oe_pwm();
	set_global_brightness(255); //0 - 255


	
	xTaskCreatePinnedToCore(refresh_display_task, "Refresh", 2048, NULL, 1, NULL, 0);
	xTaskCreatePinnedToCore(drawing_task,         "Draw",    4096, NULL, 0, NULL, 1);
	//xTaskCreatePinnedToCore(background_task,      "BG",      1024, NULL, 1, NULL, 1);


   while (1) {
		vTaskDelay(pdMS_TO_TICKS(2));

    }



  
}


