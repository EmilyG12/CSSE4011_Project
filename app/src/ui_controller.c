#include "game_controller.h"

#include <fight_ad.h>
#include "game.h"
#include "pokedex.h"
#include "bt/bluetooth.h"
#include "ui/viewer.h"
#include "ui_controller.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(app);

GameController *controller;
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

void do_nothing(void) {}

void clear(void) {
    battle_config.buttons = NULL;
    battle_config.buttonCount = 0;

    conn_config.buttons = NULL;
    conn_config.buttonCount = 0;
}

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
    clear();
    ui_controller.waiting.update = update_waiting_screen;
    ui_controller.battle.update = do_nothing;
    waitingButtons[0] = (ButtonConfig){.label = "back", 0, true, do_nothing};
    conn_config.buttons = waitingButtons;
    conn_config.buttonCount = 1;

    update_waiting_screen();
}

bool buttonsOn(void) {
        bool wentFirst = !!controller.me.player->challengee;
        Fight* f = find_fight(controller.arena->fights, controller.arena->fightCount, controller.me.player->sessionID);
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
    update_battle_scene(&battle_config);
}

PlayerDisplayConfig init_player_config(Player* player) {
    PlayerDisplayConfig config = {
        .health = get_pokemon(player->fighter)->maxHP,
        .healthMax = get_pokemon(player->fighter)->maxHP,
        .name = player->name,
        .spriteName = get_pokemon(player->fighter)->name,
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

    clear();

    ui_controller.waiting.update = do_nothing;
    ui_controller.battle.update = update_fight_screen;

    moveButtons[0] = (ButtonConfig){.label = "flee", 2, true, do_nothing};
    battle_config.buttons = moveButtons;
    battle_config.buttonCount = 1;
    for (int i = 0; i < 4; i++) {
        moveButtons[battle_config.buttonCount++] = (ButtonConfig){
            .label = get_move(controller.me.player->moves[i])->name,
            i, buttonsOn(), do_nothing};
    }

    battle_config.me = init_player_config(controller.me.player);
    battle_config.opponent = init_player_config(controller.opponent.player);

    init_battle_scene(&battle_config);
}

void button_pressed(char letter) {
// 
}

UiController *init_ui(GameController *ctrl) {
    controller = ctrl;
    ui_controller.battle = {
        .init = init_fight_screen
        .update = do_nothing
    };
    ui_controller.waiting = {
        .init = init_waiting_screen,
        .update = do_nothing
    };

    ui_controller.buttonPressed = button_pressed;
    
    return &ui_controller;
}