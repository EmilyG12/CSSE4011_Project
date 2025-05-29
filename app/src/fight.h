//
// Created by Sioryn Willett on 2025-05-18.
//

#ifndef FIGHT_H
#define FIGHT_H
#include <stdint.h>

typedef struct {
    uint32_t uuid;
    char name[16];
    int sequenceNumber;
    uint32_t challengee;
    uint32_t sessionID;
    int fighter;
    int hp;
    char moves[4];
} Player;

typedef struct {
    Player* players[2];
    uint32_t sessionID;
    Player* winner;
    int moveCount;
    int moves[16];
} Fight;

#endif //FIGHT_H
