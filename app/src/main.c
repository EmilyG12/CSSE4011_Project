//
// Created by Sioryn Willett on 2025-05-18.
//

#include <zephyr/shell/shell.h>
#include "controller.h"
#include "fight_ad.h"
#include "game_controller.h"
#include "../../libs/bt/bluetooth.h"

GameController* game;
InputController input_controller;

int cmd_fight(const struct shell* shell, int argc, char *argv[]) {
    return input_controller.command(shell, argc, argv);
}

SHELL_CMD_REGISTER(fight, NULL, "Start a fight with a nearby player", cmd_fight);

int main(void) {
    game = init_game();
    input_controller = init_input_controller(game);

    if (init_fight_bt()) {
        return 1;
    }

    if (!register_observer((Observer) {.callback = input_controller.observer})) {
        return 2;
    }

    // TODO init the push buttons to call ic.buttonPressed

    return 0;
}
