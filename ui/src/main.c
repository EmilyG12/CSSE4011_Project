#include <zephyr/drivers/display.h>
#include <lvgl.h>
#include <lvgl_mem.h>
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/kernel.h>

#include "viewer.h"
#include "rec_input.h"

LOG_MODULE_REGISTER(app);

/* size of stack area used by each thread */
#define STACKSIZE 1024

/* scheduling priority used by each thread */
#define PRIORITY 7

void set_coords(void) {

while(1) {
    uint32_t  events;

    events = k_event_wait(&coord_event, X_BIT | Y_BIT, false, K_FOREVER);
    
    change_logo_position(x_coord, y_coord);
    k_event_clear(&coord_event, X_BIT | Y_BIT);

    // if ((events & (X_BIT | Y_BIT)) == (X_BIT | Y_BIT)) {
    //     LOG_INF("received both\n");     
    //     
    // }
}
}

// K_THREAD_DEFINE(set_coords_id, STACKSIZE, set_coords, NULL, NULL, NULL,
// PRIORITY, 0, 0);

int main(void)
{
    const struct device *display_dev;

    display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
    if (!device_is_ready(display_dev)) {
        return 0;
    }

    create_screen();
    lv_timer_handler();
    display_blanking_off(display_dev);

    while (1) {
        uint32_t sleep_ms = lv_timer_handler();
        k_sleep(K_MSEC(10));
        // k_msleep(MIN(sleep_ms, INT32_MAX));
    }

    return 0;
}




// int main(void)
// {
// 	char count_str[11] = {0};
// 	const struct device *display_dev;
// 	lv_obj_t *hello_world_label;
// 	// lv_obj_t *count_label;

// 	display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
// 	if (!device_is_ready(display_dev)) {
// 		LOG_ERR("Device not ready, aborting test");
// 		return 0;
// 	}

// 	if (IS_ENABLED(CONFIG_LV_Z_POINTER_INPUT)) {
// 		lv_obj_t *hello_world_button;

// 		hello_world_button = lv_button_create(lv_screen_active());
// 		lv_obj_align(hello_world_button, LV_ALIGN_CENTER, 0, -15);
// 		lv_obj_add_event_cb(hello_world_button, lv_btn_click_callback, LV_EVENT_CLICKED,
// 				    NULL);
// 		hello_world_label = lv_label_create(hello_world_button);
// 	}

// 	lv_label_set_text(hello_world_label, "Hello world!");
// 	lv_obj_align(hello_world_label, LV_ALIGN_CENTER, 0, 0);

// 	// count_label = lv_label_create(lv_screen_active());
// 	// lv_obj_align(count_label, LV_ALIGN_BOTTOM_MID, 0, 0);

// 	lv_timer_handler();
// 	display_blanking_off(display_dev);

// 	while (1) {
// 		// if ((count % 100) == 0U) {
// 		// 	sprintf(count_str, "%d", count/100U);
// 		// 	lv_label_set_text(count_label, count_str);
// 		// }
// 		lv_timer_handler();
// 		k_sleep(K_MSEC(10));
// 	}
// }