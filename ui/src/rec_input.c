/*
* Code inspired by
* https://docs.zephyrproject.org/latest/samples/bluetooth/st_ble_sensor/README.html#bluetooth_st_ble_sensor
*/

#include <zephyr/types.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <zephyr/sys/byteorder.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/kernel.h>
#include "rec_input.h"

struct k_work_delayable adv_restart_work;
K_EVENT_DEFINE(coord_event);

/* Global Vars */
double x_coord = 0.0;
double y_coord = 0.0;

/* Write X Value Handler */
static ssize_t recvx(struct bt_conn *conn,
            const struct bt_gatt_attr *attr, const void *buf,
            uint16_t len, uint16_t offset, uint8_t flags);

/* Write Y Value Handler */
static ssize_t recvy(struct bt_conn *conn,
            const struct bt_gatt_attr *attr, const void *buf,
            uint16_t len, uint16_t offset, uint8_t flags);

/* ST Custom Service  */
static const struct bt_uuid_128 st_service_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0x0000fe40, 0xcc7a, 0x482a, 0x984a, 0x7f2ed5b3e58f));

/* ST X Value Service */
static const struct bt_uuid_128 x_value_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0x0000fe41, 0x8e22, 0x4541, 0x9d4c, 0x21edae82ed19));

/* ST Y Value Service */
static const struct bt_uuid_128 y_value_uuid = BT_UUID_INIT_128(
    BT_UUID_128_ENCODE(0x0000fe42, 0x8e22, 0x4541, 0x9d4c, 0x21edae82ed19));

/* Advertisement Information */
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),

};

/* BLE connection */
struct bt_conn *ble_conn;

/* ST GATT services and characteristic */
BT_GATT_SERVICE_DEFINE(stsensor_svc,
    BT_GATT_PRIMARY_SERVICE(&st_service_uuid),
    BT_GATT_CHARACTERISTIC(&x_value_uuid.uuid,
        BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
        BT_GATT_PERM_WRITE,
        NULL, recvx, NULL),
    BT_GATT_CHARACTERISTIC(&y_value_uuid.uuid,
        BT_GATT_CHRC_WRITE | BT_GATT_CHRC_WRITE_WITHOUT_RESP,
        BT_GATT_PERM_WRITE,
        NULL, recvy, NULL),
);

/* Receive the Y value and convert to double
*/
static ssize_t recvy(struct bt_conn *conn,
    const struct bt_gatt_attr *attr, const void *buf,
    uint16_t len, uint16_t offset, uint8_t flags)
{ 
    // Error checking
    if (len >= 20) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    if (offset != 0) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    if (!buf) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    char yCoordStr[21] = {0};

    if (len >= sizeof(yCoordStr)) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    memcpy(yCoordStr, buf, len);
    yCoordStr[len] = '\0';

    // Convert to double
    char* end = NULL;
    y_coord = strtod ( yCoordStr, &end);

    if (end == yCoordStr) {
        return 1;
    } else if (*end != '\0') {
        return 1;
    }

    k_event_post(&coord_event, Y_BIT);
    return len;
}

/* Receive the X value and convert to double
*/
static ssize_t recvx(struct bt_conn *conn,
            const struct bt_gatt_attr *attr, const void *buf,
            uint16_t len, uint16_t offset, uint8_t flags)
{ 
    // Error Checking
    if (len >= 16) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    if (offset != 0) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_OFFSET);
    }

    if (!buf) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }
    char xCoordStr[21] = {0};

    if (len >= sizeof(xCoordStr)) {
        return BT_GATT_ERR(BT_ATT_ERR_INVALID_ATTRIBUTE_LEN);
    }

    // Convert to double
    memcpy(xCoordStr, buf, len);
    xCoordStr[len] = '\0'; 

    char* end = NULL;
    x_coord = strtod ( xCoordStr, &end);

    if (end == xCoordStr) {
        return 1;
    } else if (*end != '\0') {
        return 1;
    }

    k_event_post(&coord_event, X_BIT);
    return len;
}

/* Advertise bluetooth service
*/
void bt_ready(int err)
{
    if (err) {
        return;
    }

    /* Start advertising */
    err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        return;
    }
}

/* Connection handler 
*/
static void connected(struct bt_conn *connected, uint8_t err)
{
    if (!err) {
        printk("Connected!\n");
        if (!ble_conn) {
            ble_conn = bt_conn_ref(connected);
        }
    }
}

void adv_restart_handler(struct k_work *work)
{
    int err = bt_le_adv_start(BT_LE_ADV_CONN_FAST_1, ad, ARRAY_SIZE(ad), NULL, 0);
    if (err) {
        printk("failed: %d\n", err);
    } else {
        printk("restarted.\n");
    }
}

static void disconnected(struct bt_conn *disconn, uint8_t reason)
{
    printk("Reason: %d\n", reason);

    if (ble_conn) {
        bt_conn_unref(ble_conn);
        ble_conn = NULL;
    }

    // Schedule advertising restart 300ms later
    k_work_schedule(&adv_restart_work, K_MSEC(50));
}


BT_CONN_CB_DEFINE(conn_callbacks) = {
    .connected = connected,
    .disconnected = disconnected,
};
