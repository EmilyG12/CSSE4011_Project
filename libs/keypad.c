#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(keypad);

#ifdef CONFIG_INPUT_KBD_MATRIX
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/input/input.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/shell/shell.h>
#include <zephyr/input/input_kbd_matrix.h>
#include <zephyr/dt-bindings/input/keymap.h>
#include <zephyr/input/input_keymap.h>

#define STACK_SIZE 1024
#define THREAD_PRIORITY 7


typedef struct {
    size_t x;
    size_t y;
    bool pressed;
} KeypadEvent;

KeypadEvent keypad;

static const struct device *const kp_dev = DEVICE_DT_GET(DT_NODELABEL(kbdmatrix));

void (*user_kp_callback)(char letter) = NULL;

char keys[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'0', 'F', 'E', 'D'}
};

// K_QUEUE_DEFINE(kp_queue, 10);

static void kp_callback(struct input_event *evt, void *user_data)
{  
    char letter = 'K';
    if (evt->code == INPUT_ABS_X) {
        keypad.x = evt->value;
    }
    if (evt->code == INPUT_ABS_Y) {
        keypad.y = evt->value;
    }
    if (evt->code == INPUT_BTN_TOUCH) {
        keypad.pressed = evt->value;
    }
    if (!evt->sync) {
        return;
    }

    if (keypad.pressed && keypad.x != 50 && keypad.y != 50) {
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                if (keypad.x == i && keypad.y == j) {
                    letter = keys[j][i];
                    break;
                }
            }
        }    
        char *mempad = malloc(sizeof(char));
        *mempad = letter; // Store the pressed key character
        // k_queue_append(&kp_queue, mempad);
        keypad.x = 50;
        keypad.y = 50;


        if (user_kp_callback) {
            user_kp_callback(letter); // Replace 'a' with the actual character based on your logic
        }
    }

}

// void kp_thread(void) {
//     while(true) {
//         // char *mempad = k_queue_get(&kp_queue, K_FOREVER);
//         if (user_kp_callback) {
//             user_kp_callback(*mempad);
//         }
//         free(mempad);
//     }
// }

// K_THREAD_DEFINE(kp_thread, STACK_SIZE, kp_callback, NULL, NULL, NULL,
            //   THREAD_PRIORITY, 0, K_NO_WAIT);

        // for (int i = 0; i < 4; i++) {
        //     for (int j = 0; j < 4; j++) {
        //         if (keypad.x == i && keypad.y == j) {
        //             letter = keys[j][i];
        //             LOG_INF("Key pressed: %c", letter);
        //             break;
        //         }
        //     }
        // }
        // keypad.x = 50;
        // keypad.y = 50;
// This macro registers the callback automatically.
INPUT_CALLBACK_DEFINE(kp_dev, kp_callback, NULL);

#endif // CONFIG_INPUT_KBD_MATRIX

void set_user_kp_callback(void (*callback)(char letter)) {
    #ifdef CONFIG_INPUT_KBD_MATRIX
        user_kp_callback = callback;
    #else
        LOG_WRN("No keypad available");
    #endif

}
