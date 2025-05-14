//
// Created by Sioryn Willett on 2025-04-24.
//

#include "bluetooth.h"
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>

#include "bt_advertiser.h"
#include "bt_scanner.h"

LOG_MODULE_REGISTER(bt);

Advertiser advertiser = {};
Scanner scanner = {};

void bt_ready(int err) {
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)", err);
        return;
    }
}

int init_bt() {
    return bt_enable(bt_ready);
}

bool register_advertisement(Advertisement ad) {
    if (!advertiser.add) {
        advertiser = init_advertiser();
    }

    return !advertiser.add(ad);
}

bool update_advertisements() {
    return advertiser.update ? !advertiser.update() : false;
}

bool register_observer(Observer observer) {
    if (!scanner.add) {
        scanner = init_scanner();
    }

    return !scanner.add(observer);
}
