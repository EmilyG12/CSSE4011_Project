//
// Created by Sioryn Willett on 2025-04-25.
//

#ifndef BT_OBSERVER_H
#define BT_OBSERVER_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

typedef bool(*ObserverCallback)(const char* mac_addr, int rssi, int type, uint8_t data[], size_t len);

typedef struct {
    ObserverCallback filter;
    ObserverCallback callback;
} Observer;

#endif //BT_OBSERVER_H
