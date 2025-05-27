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
ConnectionSceneConfig conn_config = {
    .buttons = NULL,
    .buttonCount = 0,
};

ButtonConfig moveButtons[9] = {};
BattleSceneConfig battle_config = {
    .buttons = NULL,
    .buttonCount = 0
};

void do_nothing(int i) {}

void update_waiting_screen(void) {
    if (!conn_config.buttons) {
        return;
    }

    conn_config.buttonCount = 1;
    for (int i = 0; i < controller.arena->pendingCount; i++) {
        if (controller.arena->pendingPlayers[i]->challengee == controller.me.player->uuid) {
            waitingButtons[conn_config.buttonCount++] = (ButtonConfig){
                .label = controller.arena->pendingPlayers[i]->name,
                .id = conn_config.buttonCount - 1,
                .callback = do_nothing,
                .on = 2
            };
        }
    }

    for (int i = 0; i < controller.arena->waitingCount; i++) {
        if (controller.arena->waitingPlayers[i]->uuid == controller.me.player->uuid) {
            continue;
        }
        waitingButtons[conn_config.buttonCount++] = (ButtonConfig){
            .label = controller.arena->waitingPlayers[i]->name,
            .id = conn_config.buttonCount - 1,
            .callback = do_nothing,
            .on = true
        };
    }

    init_connections_scene(&conn_config);
}

void init_waiting_screen(void) {
    LOG_INF("Changing to connection scene");
    battle_config.buttons = NULL;
    battle_config.buttonCount = 0;

    waitingButtons[0] = (ButtonConfig){.label = "back", 0, true, do_nothing};
    conn_config.buttons = waitingButtons;
    conn_config.buttonCount = 1;

    update_waiting_screen();
}

void update_fight_screen(void) {
    if (!battle_config.buttons) {
        return;
    }

    update_battle_scene(&battle_config);
}

PlayerDisplayConfig init_player_config(Player* player) {
    PlayerDisplayConfig config = {
        .health = player->fighter,
        .healthMax = player->fighter,
        .name = player->name,
        .spriteName = "bulbasaur",
        .turn = !!player->challengee,
        .playerNum = controller.me.player->uuid == player->uuid ? 1 : 2
    };

    return config;
}

void init_fight_screen(void) {
    LOG_INF("Changing to battle scene");
    if (!controller.me.player || !controller.opponent.player) {
        LOG_ERR("Failed to initialize battle scene");
        return;
    }

    conn_config.buttons = NULL;
    conn_config.buttonCount = 0;

    moveButtons[0] = (ButtonConfig){.label = "flee", 2, true, do_nothing};
    battle_config.buttons = moveButtons;
    battle_config.buttonCount = 1;
    for (int i = 0; i < 4; i++) {
        moveButtons[battle_config.buttonCount++] = (ButtonConfig){
            // FIXME this is a hacky thing in the meantime
            .label = "abcde" + controller.me.player->moves[i],
            i,true, do_nothing};
    }

    battle_config.me = init_player_config(controller.me.player);
    battle_config.opponent = init_player_config(controller.opponent.player);

    init_battle_scene(&battle_config);
}

int player_waiting(uint32_t uuid, uint16_t seq, const char* name) {
    int err = fight_ad_waiting(name);
    if (err){
        LOG_ERR("ad failed :\'(");
        return err;
    }

    int updated = register_waiting(uuid, seq, name);
    if (updated > 0) {
        controller.me.player = find_player_by_uuid(controller.arena->players, controller.arena->playerCount, uuid);
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
    init_fight_screen();
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

    // TODO blank battle_scene

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
        update_waiting_screen();
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
            return 0;
        }

        if (uuid != controller.me.player->challengee) {
            LOG_INF("That isn't who I challenged");
            return -1;
        }

        controller.opponent.player = find_player_by_uuid(controller.arena->players, controller.arena->playerCount, opponentUUID);
        init_fight_screen();
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
    int updated = register_fled(uuid, seq, sessionID);
    if (updated > 0) {
        // TODO unblank battle_scene
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
            .accept = opponent_accept,
            .fled = opponent_fled,
            .move = opponent_move,
        },
        .arena = get_arena(),
    };

    return &controller;
}
