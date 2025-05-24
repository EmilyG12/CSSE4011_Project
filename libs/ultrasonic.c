#include <zephyr/types.h>
#include <stddef.h>
#include <zephyr/sys/util.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <math.h>
#include <stdint.h>
#include <zephyr/devicetree.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#define SENSOR_LABEL "ULTRASONIC_0"

LOG_MODULE_REGISTER(ultrasonic, CONFIG_LOG_DEFAULT_LEVEL);

#define RESTART_INTERVAL_MS 200
#define STACK_SIZE 1024
#define THREAD_PRIORITY 5
#define MAX_WAIT_US 30000

typedef struct {
    const struct device* device;
    void (*callback)(int id, uint8_t m, uint8_t cm);
    uint8_t m;
    uint8_t cm;
} US;

US uses[2] = {
    {.device = DEVICE_DT_GET(DT_NODELABEL(ultrasonic1)), .m = 0xFF, .cm = 0xFF},
    {.device = DEVICE_DT_GET(DT_NODELABEL(ultrasonic2)), .m = 0xFF, .cm = 0xFF}
};

void split_distance(int id, float distance_cm){
    
    if (distance_cm < 0) {
        uses[id].m = 0;
        uses[id].cm = 0;
        return;
    }

    uses[id].m = (uint16_t)(distance_cm / 100.0f);
    float cm_remainder = distance_cm - (uses[id].m * 100.0f);
    uses[id].cm = (uint8_t)ceilf(cm_remainder);  // Round UP
}

void set_ultrasonic_payload(int id, uint8_t byte1, uint8_t byte2){
    uses[id].m = byte1;
    uses[id].cm = byte2;
}

int init_ultrasonic(int id, void(*callback)(int id, uint8_t metres, uint8_t cms)) {
    
    if (!device_is_ready(uses[id].device)) {
        LOG_ERR("Ultrasonic sensor %d not ready\n", id);
        return 2;
    }

    LOG_INF("Ultrasonic sensor %d ready!\n", id);

    uses[id].callback = callback;

    return 0;
}

uint8_t get_ultrasonic_metres(int id) {
  return uses[id].m;
}

uint8_t get_ultrasonic_cms(int id) {
  return uses[id].cm;
}

void ultra_thread(int id) {
    
    while (1) {   

        int ret = sensor_sample_fetch(uses[id].device);
        
        if (ret < 0) {
            LOG_ERR("Failed to fetch sample: %d", ret);
            k_msleep(200);
            continue;
        } 

        struct sensor_value distance;
        if (sensor_channel_get(uses[id].device, SENSOR_CHAN_DISTANCE, &distance) < 0) {
            LOG_ERR("Failed to get distance channel");
        }

        int total_cm = (distance.val1 * 100) + (distance.val2 / 10000);

        uint16_t meters = total_cm / 100;
        uint8_t centimeters = (uint8_t)ceilf(total_cm % 100);

        set_ultrasonic_payload(id, meters, centimeters);
        if (uses[id].callback){
            uses[id].callback(id, uses[id].m, uses[id].cm);
        }

        k_msleep(200);
    }
    return;
}

void ultra1(void) {
    ultra_thread(0);
}

void ultra2(void) {
    ultra_thread(1);
}

K_THREAD_DEFINE(ultrasonic1_tid, STACK_SIZE,
    ultra2, NULL, NULL, NULL,
    THREAD_PRIORITY, 0, 0);

    
K_THREAD_DEFINE(ultrasonic2_tid, STACK_SIZE,
    ultra1, NULL, NULL, NULL,
    THREAD_PRIORITY, 0, 0);
