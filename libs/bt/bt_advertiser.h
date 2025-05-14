//
// Created by Sioryn Willett on 2025-04-24.
//

#ifndef BT_ADVERTISEMER_H
#define BT_ADVERTISEMER_H

#include "bt_advertisement.h"

typedef struct {
    int (*add)(Advertisement advertisement);
    int (*update)(void);
} Advertiser;

Advertiser init_advertiser();

#endif //BT_ADVERTISEMER_H
