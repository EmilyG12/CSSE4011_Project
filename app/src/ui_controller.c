#include "game_controller.h"

#include <fight_ad.h>
#include "game.h"
#include "pokedex.h"
#include "bt/bluetooth.h"
#include "ui/viewer.h"
#include "ui_controller.h"
#include <zephyr/random/random.h>

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

ButtonConfig splashButtons[2] = {};
SplashSceneConfig splash_config = {
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

    splash_config.buttons = NULL;
    splash_config.buttonCount = 0;

    ui_controller.battle.update = do_nothing_void;
    ui_controller.waiting.update = do_nothing_void;
    ui_controller.splash.update = do_nothing_void;
}

void update_splash_screen(void) {
    do_nothing_void();
}

void init_splash_screen(void) {
    splashButtons[splash_config.buttonCount++] = (ButtonConfig){.label = "Connect!", 0, true, do_nothing};
    splashButtons[splash_config.buttonCount++] = (ButtonConfig){.label = "Choose!", 0, true, do_nothing};
    splash_config.buttons = splashButtons;
    ui_controller.splash.update = update_splash_screen;
    init_splash_scene(&splash_config);
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
        bool wentFirst = !!g_controller->me.player->challengee;
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

    moveButtons[0] = (ButtonConfig){.label = "flee", 2, true, do_nothing};
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
    if (ui_controller.battle.update != do_nothing_void) {
        if (buttonsOn()) {
            int move  = letter - '1';
            if (move >= 0 && move < 4) {
                g_controller->me.move (
                *get_fight_ad().uuid,
                *get_fight_ad().sequenceNumber + 1,
                *get_fight_ad().sessionID,
                letter - '1'
                );
            }
        } else if (letter == '5') {
            g_controller->me.fled (                
                *get_fight_ad().uuid,
                *get_fight_ad().sequenceNumber + 1,
                *get_fight_ad().sessionID
            );
        }
    }
    
    if (ui_controller.waiting.update != do_nothing_void) {
        int conns = letter - '1';
        if (conns >= 0 && conns < 9 && (conn_config.buttonCount <= conns)) {
            int mode = waitingButtons[conns].on;
            Player* opponent = find_player_by_name(g_controller->arena->players, g_controller->arena->playerCount, waitingButtons[conns].label);
            if (!opponent) {
                return;
            }
            if (mode == 1){
                // initiate uint32_t uuid, uint16_t seq, uint32_t opponentUUID, uint32_t sessionID, int fighter, char moves[4]
                g_controller->me.initiate (
                    g_controller->me.player->uuid, 
                    g_controller->me.player->sequenceNumber + 1,
                    opponent->uuid, 
                    sys_rand32_get(),
                    g_controller->me.player->fighter,
                    g_controller->me.player->moves
                );
                //uint32_t uuid, uint16_t seq, uint32_t opponentUUID, uint32_t sessionID, int fighter, char moves[4]
            } else if (mode == 2){
                // accept
                g_controller->me.accept (
                    g_controller->me.player->uuid, 
                    g_controller->me.player->sequenceNumber + 1,
                    opponent->uuid, 
                    opponent->sessionID, 
                    get_user()->fighter.id,
                    get_user()->fighter.moves
                );
            }
        }
    }

    if (ui_controller.splash.update != do_nothing_void) {
        if (letter == '1') {
           // uint32_t uuid, uint16_t seq, uint32_t sessionID, int i
            FightAd my_ad = get_fight_ad();
            // g_controller->me.waiting (
            //     g_controller->me.player->uuid,
            //     g_controller->me.player->sequenceNumber + 1,
            //     get_user()->name
            // );
        }
    }

    return;
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

    ui_controller.splash = (ScreenController) {
        .init = init_splash_screen,
        .update = do_nothing_void
    };
    init_screen();
    init_splash_screen();
    ui_controller.buttonPressed = ui_button_pressed;
    return &ui_controller;
}