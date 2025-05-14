//
// Created by Sioryn Willett on 2025-04-24.
//

#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/hci.h>

#include "bt_advertisement.h"
#include "bt_observer.h"

int init_bt(void);

bool register_advertisement(Advertisement ad);

bool update_advertisements();

bool register_observer(Observer observer);

#endif //BLUETOOTH_H
