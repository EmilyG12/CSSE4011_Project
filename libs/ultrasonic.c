#include <zephyr/types.h>
#include <stddef.h>
#include <zephyr/sys/util.h>
#include <zephyr/kernel.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <math.h>
#include <stdint.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>
#include <bt/bluetooth.h>

#define SENSOR_LABEL "ULTRASONIC_0"

LOG_MODULE_REGISTER(ultrasonic, CONFIG_LOG_DEFAULT_LEVEL);

#define RESTART_INTERVAL_MS 200
#define STACK_SIZE 1024
#define THREAD_PRIORITY 5
#define MAX_WAIT_US 30000

const struct device *dev = DEVICE_DT_GET(DT_NODELABEL(ultrasonic));

void(*usCallback)(uint8_t metres, uint8_t cms) = NULL;

uint8_t metres = 0xFF;
uint8_t cms = 0xFF;

void split_distance(float distance_cm, uint16_t *meters, uint8_t *centimeters){
    if (distance_cm < 0) {
        *meters = 0;
        *centimeters = 0;
        return;
    }

    *meters = (uint16_t)(distance_cm / 100.0f);
    float cm_remainder = distance_cm - (*meters * 100.0f);
    *centimeters = (uint8_t)ceilf(cm_remainder);  // Round UP
}


void set_ultrasonic_payload(uint8_t byte1, uint8_t byte2){
    metres = byte1;
    cms = byte2;
}

int init_ultrasonic(void(*callback)(uint8_t metres, uint8_t cms)){
    if (!device_is_ready(dev)) {
        LOG_ERR("Ultrasonic sensor not ready\n");
        return 2;
    }

    usCallback = callback;

    return 0;
}

uint8_t get_ultrasonic_metres(void){
  return metres;
}

uint8_t get_ultrasonic_cms(void){
  return cms;
}

void ultra_thread(void) {
    while (1) {        
        if (sensor_sample_fetch(dev) < 0) {
            LOG_ERR("Failed to fetch sample");
            k_msleep(200);
            continue;
        }

        struct sensor_value distance;
        if (sensor_channel_get(dev, SENSOR_CHAN_DISTANCE, &distance) < 0) {
            LOG_ERR("Failed to get distance channel");
        }
        int total_cm = (distance.val1 * 100) + (distance.val2 / 10000);

        uint16_t meters = total_cm / 100;
        uint8_t centimeters = (uint8_t)ceilf(total_cm % 100);

        set_ultrasonic_payload(meters, centimeters);
        if (usCallback){
            usCallback(metres, cms);
        }

        k_msleep(200);
    }
    return;
}

K_THREAD_DEFINE(ultrasonic_tid, STACK_SIZE,
    ultra_thread, NULL, NULL, NULL,
    THREAD_PRIORITY, 0, 0);


