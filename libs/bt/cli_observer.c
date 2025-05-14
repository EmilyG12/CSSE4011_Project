//
// Created by Sioryn Willett on 2025-04-24.
//
#include <stdlib.h>
#include <zephyr/shell/shell.h>
#include <zephyr/logging/log.h>
#include "bluetooth.h"
#include "bt_scanner.h"

LOG_MODULE_DECLARE(bt);

const char* defaultAddresses[] = {
    "CA:99:9E:FD:98:B1",
    "CB:1B:89:82:FF:FE",
    "D4:D2:A0:A4:5C:AC",
    "C1:13:27:E9:B7:7C",
    "F1:04:48:06:39:A0",
    "CA:0C:E0:DB:CE:60",
    NULL
};

void push_rssis_to_ad(const char* addr, int rssi) {
    BNode* nodes = get_nodes();
    int rssis[MAX_BEACONS];
    for (int i = 0; i < MAX_BEACONS; i++) {
        rssis[i] = nodes[i].rssi;
    }
//    push_data(rssis, MAX_BEACONS);

    // LOG_INF("Found target MAC %s | RSSI: %d dBm", addr_str, beacons[i].rssi);
}

int register_mac_addrs(const char* addresses[]) {
    for (int i = 0; addresses[i]; i++) {
        int err = register_mac_addr(addresses[i], push_rssis_to_ad);
        if (err) {
            LOG_ERR("Failed to register MAC address %s", addresses[i]);
            return err;
        }
    }
    return 0;
}

static int cmd_setup(const struct shell *shell, size_t argc, const char **argv) {
    return register_mac_addrs(argc < 2 ? defaultAddresses : argv + 1);
}

SHELL_CMD_REGISTER(setup, NULL, "Setup iBeacon Nodes", cmd_setup);
