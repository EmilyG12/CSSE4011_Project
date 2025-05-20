//
// Created by Sioryn Willett on 2025-04-24.
//

#include "bt_scanner.h"

#include <strings.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/logging/log.h>

LOG_MODULE_DECLARE(bt);

#define MAX_OBSERVERS 32
Observer observers[MAX_OBSERVERS];
int observerLen = 0;

int add_observer(Observer observer) {
    if (observerLen >= MAX_OBSERVERS) {
        LOG_ERR("Scanner reached observer capacity");
        return 1;
    }

    observers[observerLen++] = observer;
    return 0;
}

int add_observer_failed(Observer observer) {
    LOG_ERR("Bluetooth scanner not initialised!");
    return 1;
}

/**
 * Handle receiving a bluetooth advertisement.
 * If it's a registered device record it's rssi.
 *
 * @param addr  Bluetooth address of the advertised device.
 * @param rssi  Current rssi from the advertised device.
 * @param type  Bluetooth type for the advertised device.
 * @param ad    The net buffer.
 */
static void device_found(const bt_addr_le_t *addr, int8_t rssi, uint8_t type, struct net_buf_simple *ad) {
    char addr_str[BT_ADDR_LE_STR_LEN];
    bt_addr_le_to_str(addr, addr_str, sizeof(addr_str));
    strtok(addr_str, " "); // Trimming everything after first whitespace.

    for (int i = 0; i < observerLen; i++) {
        if (observers[i].filter) {
            if (!observers[i].filter(addr_str, rssi, type, ad->data, ad->len)) {
                continue;
            }
        }

        LOG_DBG("Observer Matched: %s", addr_str);

        if (observers[i].callback) {
            observers[i].callback(addr_str, rssi, type, ad->data, ad->len);
        }
    }
}

// See header
Scanner init_scanner(void) {
    struct bt_le_scan_param scan_param = {
        .type = BT_LE_SCAN_TYPE_PASSIVE,
        .options = BT_LE_SCAN_OPT_NONE,
        .interval = BT_GAP_SCAN_FAST_INTERVAL,
        .window = BT_GAP_SCAN_FAST_WINDOW,
    };

    int err = bt_le_scan_start(&scan_param, device_found);
    if (err) {
        LOG_ERR("Start scanning failed (err %d)", err);
        return (Scanner){.add = add_observer_failed};
    }

    LOG_INF("Started scanning...");
    return (Scanner){.add = add_observer};
}