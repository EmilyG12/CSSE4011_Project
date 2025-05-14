//
// Created by Sioryn Willett on 2025-04-25.
//

#ifndef BT_ADVERTISEMENT_H
#define BT_ADVERTISEMENT_H

#include <stdint.h>
#include <stdlib.h>

typedef struct {
    uint8_t type;
    uint8_t* data;
    size_t len;
} Advertisement;

#endif //BT_ADVERTISEMENT_H
