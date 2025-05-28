//
// Created by Sioryn Willett on 2025-05-27.
//

#include "event.h"
#include <zephyr/kernel.h>

#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);

K_QUEUE_DEFINE(my_thingo);

int main(void){
    k_queue_init(&my_thingo);

    char bob[] = "some shit";
    LOG_INF("%s", bob);
    // for (int i = 0; i < sizeof(bob) - 1; i++) {
    //     k_queue_append(&my_thingo, bob + i);
    // }

    k_queue_append(&my_thingo, bob);

    while (!k_queue_is_empty(&my_thingo)) {
        char* ad = k_queue_get(&my_thingo, K_MSEC(10));
        if (!ad) {
            return 1;
        }
        LOG_INF("main: (%c)", *ad);
    }

    LOG_INF("%s done!", bob);

    return 0;
}