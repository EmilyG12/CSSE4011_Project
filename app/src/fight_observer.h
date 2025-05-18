//
// Created by Sioryn Willett on 2025-05-18.
//

#ifndef FIGHT_OBSERVER_H
#define FIGHT_OBSERVER_H
#include "../../libs/bt/bt_observer.h"

bool fight_ad_observe_arena(const char* mac_addr, int rssi, int type, uint8_t data[], size_t len);

#endif //FIGHT_OBSERVER_H
