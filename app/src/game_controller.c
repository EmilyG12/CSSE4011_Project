//
// Created by Sioryn Willett on 2025-05-18.
//

#include "game_controller.h"

#include <fight_ad.h>
#include "game.h"
#include "bt/bluetooth.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(app);

GameController controller;

int player_waiting(uint32_t uuid, uint16_t seq, const char* name) {
    int err = fight_ad_waiting(name);
    if (err){
        LOG_ERR("ad failed :\'(");
        return err;
    }
    return register_waiting(uuid, seq, name);
}

int player_initiate(uint32_t uuid, uint16_t seq, uint32_t opponentUUID, uint32_t sessionID, int fighter, char moves[4]) {
    int err = fight_ad_initiate(opponentUUID, sessionID, fighter, moves);
    if (err){
        LOG_ERR("ad failed :\'(");
        return err;
    }
    return register_initiate(uuid, seq, opponentUUID, sessionID, fighter, moves);
}

int player_accept(uint32_t uuid, uint16_t seq, uint32_t opponentUUID, uint32_t sessionID, int fighter, char moves[4]) {
    int err = fight_ad_accept(opponentUUID, sessionID, fighter, moves);
    if (err){
        LOG_ERR("ad failed :\'(");
        return err;
    }
    return register_accept(uuid, seq, opponentUUID, sessionID, fighter, moves);
}

int player_fled(uint32_t uuid, uint16_t seq, uint32_t sessionID) {
    int err = fight_ad_flee();
    if (err){
        LOG_ERR("ad failed :\'(");
        return err;
    }
    return register_fled(uuid, seq, sessionID);
}

int player_move(uint32_t uuid, uint16_t seq, uint32_t sessionID, int i) {
    int err = fight_ad_move(i);
    if (err){
        LOG_ERR("ad failed :\'(");
        return err;
    }
    return register_move(uuid, seq, sessionID, i);
}

GameController *init_game(void) {
    controller = (GameController){
        {
            .waiting = player_waiting,
            .initiate = player_initiate,
            .accept = player_accept,
            .fled = player_fled,
            .move = player_move,
        },
        {
            .waiting = register_waiting,
            .initiate = register_initiate,
            .accept = register_accept,
            .fled = register_fled,
            .move = register_move,
        }
    };

    return &controller;
}
