//
// Created by Sioryn Willett on 2025-05-18.
//

#include <user.h>
#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include "controller.h"
#include "fight_ad.h"
#include "game_controller.h"
#include "../../libs/bt/bluetooth.h"
#include "ui/viewer.h"
#include "../../libs/keypad.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);

GameController* game;
InputController input_controller;

int cmd_fight(const struct shell* shell, int argc, char *argv[]) {
    return input_controller.command(shell, argc - 1, argv + 1);
}

SHELL_CMD_REGISTER(fight, NULL, "Start a fight with a nearby player", cmd_fight);
SHELL_CMD_REGISTER(figth, NULL, "Start a fight with a nearby player", cmd_fight);


void pingu(int id) {
    LOG_INF("Pingu: %d", id);
}

int main(void) {
    set_user_name("debbie");
    set_user_fighter(1, (char[]){2, 3, 4, 5});

    if (init_fight_bt()) {
        LOG_ERR("Failed to initialize fight ad");
        return 1;
    }

    game = init_game();
    input_controller = init_input_controller(game);

    if (!register_observer((Observer) {.filter = NULL, .callback = input_controller.observer})) {
        LOG_ERR("Failed to initialise bt input");
        return 2;
    }

    init_screen();
    set_user_kp_callback(input_controller.buttonPressed);

    // FIXME make this a real splash screen
    ButtonConfig buttons[] = {
        {.label = "button 1", .id = 1, .on = true, .callback = pingu},
        {.label = "button 2", .id = 2, .on = 2, .callback = pingu},
    };
    ConnectionSceneConfig config = {
        .buttons = buttons, .buttonCount = ARRAY_SIZE(buttons)
    };
    init_connections_scene(&config);

    while (true) {
        process_queue();
        int next = update_screen();
        k_msleep(next);
    }
}
