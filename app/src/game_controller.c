//
// Created by Sioryn Willett on 2025-05-18.
//

#include "game_controller.h"

#include <stdlib.h>

GameController controller;

int game_button_pressed(int id) {
    return 0;
}

int opponent_waiting(uint32_t opponentUUID, uint32_t sessionID, const char* name) {
    return 0;
}

GameController *init_game(void) {
    controller = (GameController){
        {
            .buttonPressed = game_button_pressed,
            .waiting = NULL,
            .fled = NULL,
            .move = NULL,
        },
        {
            .buttonPressed = game_button_pressed,
            .waiting = opponent_waiting,
            .fled = NULL,
            .move = NULL,
        }
    };

    return &controller;
}
