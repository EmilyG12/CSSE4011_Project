//
// Created by Sioryn Willett on 2025-05-18.
//

#include "controller.h"

#include <zephyr/bluetooth/gap.h>

#include "fight_ad.h"
#include "fight_observer.h"

GameController *game_controller = NULL;

int process_cmd(const struct shell *shell, int argc, char **argv) {
    if (!game_controller) {
        shell_print(shell, "Game has not been initialised");
        return 1;
    }

    return 0;
}

void button_pressed(int i) {
    if (!game_controller || !game_controller->me.buttonPressed) {
        return;
    }

    game_controller->me.buttonPressed(i);
}

void fight_ad_process(FightAd ad) {
    if (*ad.command != FC_WAITING) {
        if (*ad.sessionID != *get_fight_ad().sessionID) {
            return;
        }
    }

    PlayerController opponent = game_controller->opponent;
    switch (*ad.command) {
        case FC_WAITING:
            if (opponent.waiting) {
                opponent.waiting(*ad.uuid, *ad.sessionID, (const char *) ad.args);
            }
            return;
        case FC_FLEE:
            if (opponent.fled) {
                opponent.fled();
            }
            return;
        case FC_MOVE_0: case FC_MOVE_1: case FC_MOVE_2: case FC_MOVE_3:
            if (opponent.move) {
                opponent.move(*ad.command - FC_MOVE_0);
            }
        default:
    }
}

bool fight_ad_observe(const char *mac_addr, int rssi, int type, uint8_t data[], size_t len) {
    if (!game_controller) {
        return false;
    }

    if (type != BT_DATA_MANUFACTURER_DATA) {
        return false;
    }

    FightAd fight_ad = parse_fight_ad(data, len);
    if (!fight_ad.uuid) {
        return false;
    }

    fight_ad_process(fight_ad);
    return true;
}

InputController init_input_controller(GameController *controller) {
    game_controller = controller;

    return (InputController){
        .command = process_cmd,
        .buttonPressed = button_pressed,
        .observer = fight_ad_observe
    };
}
