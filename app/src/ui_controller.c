#include "game_controller.h"

#include <fight_ad.h>
#include "game.h"
#include "pokedex.h"
#include "bt/bluetooth.h"
#include "ui/viewer.h"
#include "ui_controller.h"

#include <user.h>
#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(app);

GameController *g_controller;
UiController ui_controller;


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

void do_nothing_void(void) {

}

void clear(void) {
    battle_config.buttons = NULL;
    battle_config.buttonCount = 0;

    conn_config.buttons = NULL;
    conn_config.buttonCount = 0;

    ui_controller.battle.update = do_nothing_void;
    ui_controller.waiting.update = do_nothing_void;
}

void update_waiting_screen(void) {
    if (!conn_config.buttons) {
        return;
    }

    conn_config.buttonCount = 1;
    for (int i = 0; i < g_controller->arena->pendingCount; i++) {
        if (g_controller->arena->pendingPlayers[i]->challengee == g_controller->me.player->uuid) {
            waitingButtons[conn_config.buttonCount++] = (ButtonConfig){
                .label = g_controller->arena->pendingPlayers[i]->name,
                .id = conn_config.buttonCount - 1,
                .callback = do_nothing,
                .on = 2
            };
        }
    }

    for (int i = 0; i < g_controller->arena->waitingCount; i++) {
        if (g_controller->arena->waitingPlayers[i]->uuid == g_controller->me.player->uuid) {
            continue;
        }
        waitingButtons[conn_config.buttonCount++] = (ButtonConfig){
            .label = g_controller->arena->waitingPlayers[i]->name,
            .id = conn_config.buttonCount - 1,
            .callback = do_nothing,
            .on = true
        };
    }

    init_connections_scene(&conn_config);
}

void init_waiting_screen(void) {
    LOG_INF("Changing to connection scene");
    clear();
    ui_controller.waiting.update = update_waiting_screen;
    waitingButtons[0] = (ButtonConfig){.label = "back", 0, true, do_nothing};
    conn_config.buttons = waitingButtons;
    conn_config.buttonCount = 1;

    update_waiting_screen();
}

bool buttonsOn(void) {
        bool wentFirst = !g_controller->me.player->challengee;
        Fight* f = find_fight(g_controller->arena->fights, g_controller->arena->fightCount, g_controller->me.player->sessionID);
        if (!f) {
            return false;   
        }
        return (f->moveCount % 2) == wentFirst;
}

void update_fight_screen(void) {
    if (!battle_config.buttons) {
        return;
    }
    for (int i = 1; i < battle_config.buttonCount; i++) {
        battle_config.buttons[i].on = buttonsOn();
    }

    battle_config.me.health = g_controller->me.player->hp;
    battle_config.opponent.health = g_controller->opponent.player->hp;

    update_battle_scene(&battle_config);
}

PlayerDisplayConfig init_player_config(Player* player) {
    PlayerDisplayConfig config = {
        .health = get_pokemon(player->fighter)->maxHP,
        .healthMax = get_pokemon(player->fighter)->maxHP,
        .name = player->name,
        .spriteName = get_pokemon(player->fighter)->name,
        .turn = !!player->challengee,
        .playerNum = g_controller->me.player->uuid == player->uuid ? 1 : 2
    };

    return config;
}

void init_fight_screen(void) {
    LOG_INF("Changing to battle scene");
    if (!g_controller->me.player || !g_controller->opponent.player) {
        LOG_ERR("Failed to initialize battle scene");
        return;
    }

    clear();
    ui_controller.battle.update = update_fight_screen;

    moveButtons[0] = (ButtonConfig){.label = "flee", -1, true, do_nothing};
    battle_config.buttons = moveButtons;
    battle_config.buttonCount = 1;
    for (int i = 0; i < 4; i++) {
        moveButtons[battle_config.buttonCount++] = (ButtonConfig){
            .label = get_move(g_controller->me.player->moves[i])->name,
            i, buttonsOn(), do_nothing};
    }

    battle_config.me = init_player_config(g_controller->me.player);
    battle_config.opponent = init_player_config(g_controller->opponent.player);

    init_battle_scene(&battle_config);
}

void ui_button_pressed(char letter) {
    if (ui_controller.battle.update && buttonsOn()) {
        int move  = letter - '1';
        if (move >= 0 && move < 4) {
            g_controller->me.move(
                *get_fight_ad().uuid,
                *get_fight_ad().sequenceNumber + 1,
                *get_fight_ad().sessionID,
                letter - '1');
        }
    }
}

UiController *init_ui(GameController *ctrl) {
    g_controller = ctrl;
    ui_controller.battle = (ScreenController){
        .init = init_fight_screen,
        .update = do_nothing_void
    };
    ui_controller.waiting = (ScreenController) {
        .init = init_waiting_screen,
        .update = do_nothing_void
    };

    ui_controller.buttonPressed = ui_button_pressed;
    
    return &ui_controller;
}