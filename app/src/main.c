//
// Created by Sioryn Willett on 2025-05-18.
//

#include <fight_observer.h>
#include <zephyr/shell/shell.h>
#include "controller.h"
#include "fight_ad.h"
#include "game_controller.h"
#include "../../libs/bt/bluetooth.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);

GameController* game;
InputController input_controller;

int cmd_fight(const struct shell* shell, int argc, char *argv[]) {
    return input_controller.command(shell, argc, argv);
}

SHELL_CMD_REGISTER(fight, NULL, "Start a fight with a nearby player", cmd_fight);


int cmd_wait(const struct shell* shell, int argc, char *argv[]) {
    return fight_ad_waiting(argv[1]);
}

SHELL_CMD_ARG_REGISTER(wait, NULL, "Announce your intention to fight", cmd_wait, 2, 0);

int cmd_initiate(const struct shell* shell, int argc, char *argv[]) {
    char moves[] = {atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6])};
    return fight_ad_initiate(atoi(argv[1]), atoi(argv[2]), moves);
}

SHELL_CMD_ARG_REGISTER(initiate, NULL, "Initiate a fight with a nearby player", cmd_initiate, 7, 0);

int cmd_accept(const struct shell* shell, int argc, char *argv[]) {
    char moves[] = {atoi(argv[4]), atoi(argv[5]), atoi(argv[6]), atoi(argv[7])};
    return fight_ad_accept(strtol((argv[1]), NULL, 0) , strtol((argv[2]), NULL, 0), atoi(argv[3]), moves);
}

SHELL_CMD_ARG_REGISTER(accept, NULL, "Accept a fight from a nearby player", cmd_accept, 8, 0);


int cmd_decline(const struct shell* shell, int argc, char *argv[]) {
    return fight_ad_decline(atoi(argv[1]), atoi(argv[2]));
}

SHELL_CMD_ARG_REGISTER(decline, NULL, "Decline a fight from a nearby player", cmd_decline, 3, 0);


int cmd_flee(const struct shell* shell, int argc, char *argv[]) {
    return fight_ad_flee();
}

SHELL_CMD_ARG_REGISTER(flee, NULL, "Flee an ongoing fight", cmd_flee, 1, 0);


int cmd_move(const struct shell* shell, int argc, char *argv[]) {
    return fight_ad_move(atoi(argv[1]));
}

SHELL_CMD_ARG_REGISTER(move, NULL, "Make a move in an ongoing fight", cmd_move, 2, 0);

int main(void) {
    game = init_game();
    input_controller = init_input_controller(game);

    if (init_fight_bt()) {
        LOG_ERR("Failed to initialize fight ad");
        return 1;
    }

    if (!register_observer((Observer) {.filter = NULL, .callback = input_controller.observer})) {
        LOG_ERR("Failed to initialise bt input");
        return 2;
    }

    if (!register_observer((Observer) {.filter = NULL, .callback = fight_ad_observe_arena})) {
        LOG_ERR("Failed to initialise bt observers");
        return 3;
    }

    // TODO init the push buttons to call ic.buttonPressed

    return 0;
}
