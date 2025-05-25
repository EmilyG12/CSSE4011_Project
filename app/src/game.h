//
// Created by Sioryn Willett on 2025-05-19.
//

#ifndef GAME_H
#define GAME_H
#include <stdbool.h>
#include <stdint.h>
#include "fight.h"

typedef struct {
    int playerCount;
    Player** players;
    int waitingCount;
    Player** waitingPlayers;
    int pendingCount;
    Player** pendingPlayers;
    int fightCount;
    Fight* fights;
} Arena;

int register_waiting(uint32_t uuid, uint16_t seq, const char* name);
int register_initiate(uint32_t uuid, uint16_t seq, uint32_t opponentUUID, uint32_t sessionID, int fighter, char moves[4]);
int register_accept(uint32_t uuid, uint16_t seq, uint32_t opponentUUID, uint32_t sessionID, int fighter, char moves[4]);
int register_fled(uint32_t uuid, uint16_t seq, uint32_t sessionID);
int register_move(uint32_t uuid, uint16_t seq, uint32_t sessionID, int i);

#endif //GAME_H
