//
// Created by Sioryn Willett on 2025-05-18.
//

#include "game_controller.h"

#include <fight_ad.h>
#include "game.h"
#include "pokedex.h"
#include "bt/bluetooth.h"
#include "ui/viewer.h"
#include "ui_controller.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(app);

GameController controller;
UiController *ui;


int player_waiting(uint32_t uuid, uint16_t seq, const char* name) {
    int err = fight_ad_waiting(name);
    if (err){
        LOG_ERR("ad failed :\'(");
        return err;
    }

    int updated = register_waiting(uuid, seq, name);
    if (updated > 0) {
        controller.me.player = find_player_by_uuid(controller.arena->players, controller.arena->playerCount, uuid);
        ui->waiting.init();
        // init_waiting_screen();
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
    int updated = register_accept(uuid, seq, opponentUUID, sessionID, fighter, moves);
    if (updated < 0){
        LOG_ERR("register accept failed");
        return updated;
    }

    int err = fight_ad_accept(opponentUUID, sessionID, fighter, moves);
    if (err){
        LOG_ERR("ad failed :\'(");
        return err;
    }

    controller.opponent.player = find_player_by_uuid(controller.arena->players, controller.arena->playerCount, opponentUUID);
    ui->battle.init();
    return 0;
}

int player_fled(uint32_t uuid, uint16_t seq, uint32_t sessionID) {
    int err = fight_ad_flee();
    if (err){
        LOG_ERR("ad failed :\'(");
        return err;
    }

    // TODO go back splash screen

    return register_fled(uuid, seq, sessionID);
}

int player_move(uint32_t uuid, uint16_t seq, uint32_t sessionID, int i) {
    int err = fight_ad_move(i);
    if (err){
        LOG_ERR("ad failed :\'(");
        return err;
    }

    ui->battle.update();

    return register_move(uuid, seq, sessionID, i);
}

int opponent_waiting(uint32_t uuid, uint16_t seq, const char* name) {
    int updated = register_waiting(uuid, seq, name);
    if (updated > 0) {
        ui->waiting.update();
    }
    return updated;
}

int opponent_initiate(uint32_t uuid, uint16_t seq, uint32_t opponentUUID, uint32_t sessionID, int fighter, char moves[4]) {
    int updated = register_initiate(uuid, seq, opponentUUID, sessionID, fighter, moves);
    if (updated > 0) {
        ui->waiting.update();
    }
    return updated;
}

int opponent_accept(uint32_t uuid, uint16_t seq, uint32_t opponentUUID, uint32_t sessionID, int fighter, char moves[4]) {
    int updated = register_accept(uuid, seq, opponentUUID, sessionID, fighter, moves);
    if (updated > 0) {
        if (!controller.me.player) {
            LOG_INF("I'm not waiting");
            return 0;
        }

        if (opponentUUID != controller.me.player->uuid) {
            LOG_INF("that challenge isn't for me");
            ui->waiting.update();
            return 0;
        }

        if (uuid != controller.me.player->challengee) {
            LOG_INF("That isn't who I challenged");
            return -1;
        }

        controller.opponent.player = find_player_by_uuid(controller.arena->players, controller.arena->playerCount, uuid);
        ui->battle.init();
    }

    return updated;
}

int opponent_fled(uint32_t uuid, uint16_t seq, uint32_t sessionID){
    int updated = register_fled(uuid, seq, sessionID);
    if (updated > 0) {
        // TODO go to splash screen
    }
    return updated;
}

int opponent_move(uint32_t uuid, uint16_t seq, uint32_t sessionID, int i){
    int updated = register_move(uuid, seq, sessionID, i);
    if (updated > 0) {
        Player* myOpponent = controller.opponent.player;
        if (!myOpponent) {
            LOG_DBG("I'm not fighting");
            return 0;
        }

        if (myOpponent->uuid != uuid) {
            LOG_DBG("This isn't your opponent");
            return 0;
        }

        if (controller.me.player->sessionID != sessionID) {
            LOG_DBG("This is your opponent but it's a different fight");
            return 0;
        }

        ui->battle.update();
    }
    return updated;
}

void game_button_pressed(char letter) {
    ui->buttonPressed(letter);
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
            .accept = opponent_accept,
            .fled = opponent_fled,
            .move = opponent_move,
        },
        .arena = get_arena(),
        .button_pressed = game_button_pressed,
    };

    //init
    ui = init_ui(&controller);

    return &controller;
}
