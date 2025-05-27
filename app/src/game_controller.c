//
// Created by Sioryn Willett on 2025-05-18.
//

#include "game_controller.h"

#include <fight_ad.h>
#include "game.h"
#include "bt/bluetooth.h"
#include "ui/viewer.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(app);

GameController controller;
ButtonConfig waitingButtons[9] = {};
ConnectionSceneConfig config = {
    .buttons = NULL,
    .buttonCount = 0,
};

void do_nothing(int i) {}

void update_waiting_screen(void) {
    if (!config.buttons) {
        return;
    }

    config.buttonCount = 1;
    for (int i = 0; i < controller.arena->pendingCount; i++) {
        if (controller.arena->pendingPlayers[i]->challengee == controller.me.player->uuid) {
            waitingButtons[config.buttonCount++] = (ButtonConfig){
                .label = controller.arena->waitingPlayers[i]->name,
                .id = config.buttonCount - 1,
                .callback = do_nothing,
                .on = 2
            };
        }
    }


    for (int i = 0; i < controller.arena->waitingCount; i++) {
        if (controller.arena->waitingPlayers[i]->uuid == controller.me.player->uuid) {
            continue;
        }
        waitingButtons[config.buttonCount++] = (ButtonConfig){
            .label = controller.arena->waitingPlayers[i]->name,
            .id = config.buttonCount - 1,
            .callback = do_nothing,
            .on = true
        };
    }

    init_connections_scene(&config);
}

void init_waiting_screen(void) {
    LOG_INF("Changing to connection scene");
    waitingButtons[0] = (ButtonConfig){.label = "back", 0, true, do_nothing};
    config.buttons = waitingButtons;
    config.buttonCount = 1;
    update_waiting_screen();
}

int player_waiting(uint32_t uuid, uint16_t seq, const char* name) {
    int err = fight_ad_waiting(name);
    if (err){
        LOG_ERR("ad failed :\'(");
        return err;
    }

    int updated = register_waiting(uuid, seq, name);
    if (updated > 0) {
        controller.me.player = find_player(controller.arena->players, controller.arena->playerCount, uuid);
        init_waiting_screen();
    }
    return err;
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

int opponent_waiting(uint32_t uuid, uint16_t seq, const char* name) {
    int updated = register_waiting(uuid, seq, name);
    if (updated > 0) {
        update_waiting_screen();
    }
    return updated;
}

int opponent_initiate(uint32_t uuid, uint16_t seq, uint32_t opponentUUID, uint32_t sessionID, int fighter, char moves[4]) {
    int updated = register_initiate(uuid, seq, opponentUUID, sessionID, fighter, moves);
    if (updated > 0) {
        // update_waiting_screen();
    }
    return updated;
}

GameController *init_game(void) {
    controller = (GameController){
        .me = {
            .waiting = player_waiting,
            .initiate = player_initiate,
            .accept = player_accept,
            .fled = player_fled,
            .move = player_move,
        },
        .opponent ={
            .waiting = opponent_waiting,
            .initiate = opponent_initiate,
            .accept = register_accept,
            .fled = register_fled,
            .move = register_move,
        },
        .arena = get_arena(),
    };

    return &controller;
}
