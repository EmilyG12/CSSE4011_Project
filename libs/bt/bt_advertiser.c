//
// Created by Sioryn Willett on 2025-04-24.
//

#include "bt_advertiser.h"

#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/logging/log.h>
#include "bluetooth.h"

LOG_MODULE_DECLARE(bt);
#if CONFIG_BT_EXT_ADV
static struct bt_le_ext_adv *adv;
#endif
// Maximum number of advertisements allowed
#define MAX_ADVERTISEMENTS 4
int adsLength = 0;
struct bt_data ads[MAX_ADVERTISEMENTS];

// insert an ad into the data structure and send it to the bt library
int insert_advertisement(Advertisement ad){
    if (adsLength >= MAX_ADVERTISEMENTS) {
      LOG_ERR("Too many advertisements");
      return 1;
    }

    ads[adsLength++] = (struct bt_data){
        .type = ad.type,
        .data = ad.data,
        .data_len = ad.len
    };

    return update_advertisements();
}

// report an uninitialised advertiser
int insert_advertisement_failed(Advertisement ad){
    LOG_ERR("Advertiser not initialised");
    return 1;
}

int update_advertisement(void) {
#if CONFIG_BT_EXT_ADV
    int err = bt_le_ext_adv_set_data(adv, ads, adsLength, NULL, 0);
#else
    int err = bt_le_adv_update_data(ads, adsLength, NULL, 0);
#endif
    if (err){
        LOG_ERR("Failed to set advertisement data (err %d)", err);
    }
    return err;
}

// Create the advertiser
int create_advertiser(void){
    int err = 0;
#if CONFIG_BT_EXT_ADV
    struct bt_le_adv_param adv_params = {
        .options = BT_LE_ADV_OPT_USE_IDENTITY | BT_LE_ADV_OPT_EXT_ADV,
        .interval_min = BT_GAP_ADV_FAST_INT_MIN_2,
        .interval_max = BT_GAP_ADV_FAST_INT_MAX_2,
        .id = BT_ID_DEFAULT,
    };
    err = bt_le_ext_adv_create(&adv_params, NULL, &adv);
#endif
    if (err){
        LOG_ERR("Failed to create advertiser (err %d)", err);
    }
    return err;
}

// start advertising
int start_advertiser(void){
#if CONFIG_BT_EXT_ADV
    int err = bt_le_ext_adv_start(adv, BT_LE_EXT_ADV_START_DEFAULT);
#else
    struct bt_le_adv_param adv_params = {
        .options = BT_LE_ADV_OPT_USE_IDENTITY,
        .interval_min = BT_GAP_ADV_FAST_INT_MIN_2,
        .interval_max = BT_GAP_ADV_FAST_INT_MAX_2,
        .id = BT_ID_DEFAULT,
    };
    int err = bt_le_adv_start(&adv_params,ads, adsLength,NULL,0);
#endif

    if (err) {
        LOG_ERR("Failed to start advertiser (err %d)", err);
    }

    return err;
}

Advertiser invalid_advertiser(){
  return (Advertiser){.add = insert_advertisement_failed};
}

Advertiser valid_advertiser(){
    return (Advertiser){.add = insert_advertisement, .update = update_advertisement};
}

Advertiser init_advertiser(){
    int err = create_advertiser();
    if (err) {
        return invalid_advertiser();
    }

    err = insert_advertisement((Advertisement){
        .type = BT_DATA_FLAGS,
        .data = (uint8_t[]){BT_LE_AD_NO_BREDR},
        .len = 1
    });
    if (err) {
        return invalid_advertiser();
    }

    err = start_advertiser();
    if (err) {
        return invalid_advertiser();
    }

    return valid_advertiser();
}
