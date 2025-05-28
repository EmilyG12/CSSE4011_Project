//
// Created by Sioryn Willett on 2025-05-18.
//

#ifndef GAME_CONTROLLER_H
#define GAME_CONTROLLER_H
#include <fight.h>
#include <game.h>
#include <stdint.h>

typedef struct {
    Player* player;
    int (*waiting)(uint32_t uuid, uint16_t seq, const char *name);
    int (*initiate)(uint32_t uuid, uint16_t seq, uint32_t opponentUUID, uint32_t sessionID, int fighter, char moves[4]);
    int (*accept)(uint32_t uuid, uint16_t seq, uint32_t opponentUUID, uint32_t sessionID, int fighter, char moves[4]);
    int (*move)(uint32_t uuid, uint16_t seq, uint32_t sessionID, int i);
    int (*fled)(uint32_t uuid, uint16_t seq, uint32_t sessionID);
} PlayerController;

typedef struct {
    PlayerController me;
    PlayerController opponent;
    void(*button_pressed)(char letter);
    Arena* arena;
} GameController;

GameController *init_game(void);

#endif //GAME_CONTROLLER_H
