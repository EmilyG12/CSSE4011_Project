//
// Created by Sioryn Willett on 2025-05-18.
//

#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H
#include <stdint.h>

typedef struct {
    int (*buttonPressed)(int id);
    int (*waiting)(uint32_t opponentUUID, uint32_t sessionID, const char* name);
    int (*move)(int i);
    int (*fled)(void);
} PlayerController;

typedef struct {
    PlayerController me;
    PlayerController opponent;
} GameController;

GameController* init_game(void);

#endif //GAME_CONTROLLER_H
