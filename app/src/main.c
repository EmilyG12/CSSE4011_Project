//
// Created by Sioryn Willett on 2025-05-18.
//

#include <zephyr/shell/shell.h>
#include "controller.h"
#include "fight_ad.h"
#include "game_controller.h"
#include "../../libs/bt/bluetooth.h"
#include "ui/viewer.h"

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(app, LOG_LEVEL_DBG);

GameController* game;
InputController input_controller;

int cmd_fight(const struct shell* shell, int argc, char *argv[]) {
    return input_controller.command(shell, argc - 1, argv + 1);
}

SHELL_CMD_REGISTER(fight, NULL, "Start a fight with a nearby player", cmd_fight);


void pingu(int id) {
    LOG_INF("Pingu: %d", id);
}

int main(void) {
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

    // TODO init the push buttons to call ic.buttonPressed

    init_screen();
    ButtonConfig buttons[] = {
        {.label = "button 1", .id = 1, .on = true, .callback = pingu},
        {.label = "button 2", .id = 2, .on = true, .callback = pingu},
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
