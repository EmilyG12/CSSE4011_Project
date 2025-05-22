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
    int fighter;
    char moves[4];
} Player;

typedef struct {
    Player players[2];
    uint32_t sessionID;
    int moves[];
} Fight;

#endif //FIGHT_H
