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
        // uint32_t sleep_ms = lV_timer_handler();
        lv_timer_handler();
        k_sleep(K_MSEC(10));
    }

    return 0;
}