#define LV_CONF_INCLUDE_SIMPLE 1

#include <stdio.h>
#include "lv_conf.h"
#include <lvgl.h>
#define _DEFAULT_SOURCE /* needed for usleep() */
#include <stdlib.h>
#include <unistd.h>
#define EXAMPLE_LCD_H_RES 128
#define EXAMPLE_LCD_V_RES 64

static lv_display_t * hal_init(int32_t w, int32_t h);
extern void startup(bool);



void lvgl_setup()
{
	/*Initialize LVGL*/
		  lv_init();

		  /*Initialize the display, and the input devices*/
		  hal_init(EXAMPLE_LCD_H_RES, EXAMPLE_LCD_V_RES);

}
void main(int argc, char **argv)
{
	startup(argc == 1);

	  while(1) {
	      /* Periodically call the lv_task handler.
	       * It could be done in a timer interrupt or an OS task too.*/
	      //lv_timer_handler();
	      usleep(10* 1000);
	  }
}


static lv_display_t * hal_init(int32_t w, int32_t h)
{
  lv_group_set_default(lv_group_create());
  lv_display_t * disp = lv_sdl_window_create(w, h);
  lv_display_set_default(disp);

#if 0
  lv_indev_t * mouse = lv_sdl_mouse_create();
  lv_indev_set_group(mouse, lv_group_get_default());
  lv_indev_set_display(mouse, disp);


  (LV_IMAGE_DECLARE(mouse_cursor_icon); /*Declare the image file.*/
  lv_obj_t * cursor_obj;
  cursor_obj = lv_image_create(lv_screen_active()); /*Create an image object for the cursor */
  lv_image_set_src(cursor_obj, &mouse_cursor_icon);           /*Set the image source*/
  lv_indevl_set_cursor(mouse, cursor_obj);             /*Connect the image  object to the driver*/

  lv_indev_t * mousewheel = lv_sdl_mousewheel_create();
  lv_indev_set_display(mousewheel, disp);
#endif

  lv_indev_t * keyboard = lv_sdl_keyboard_create();
  lv_indev_set_display(keyboard, disp);
  lv_indev_set_group(keyboard, lv_group_get_default());
  lv_group_add_obj(lv_group_get_default(),lv_scr_act());
  return disp;
}
