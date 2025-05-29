//
// Created by Sioryn Willett on 2025-05-19.
//

#ifndef GAME_H
#define GAME_H
#include <stdint.h>
#include "fight.h"

#define MAX_PLAYERS 32

typedef struct {
    int playerCount;
    Player* players[MAX_PLAYERS];
    int waitingCount;
    Player* waitingPlayers[MAX_PLAYERS / 2];
    int pendingCount;
    Player* pendingPlayers[MAX_PLAYERS / 2];
    int fightCount;
    Fight fights[MAX_PLAYERS / 4];
} Arena;

Arena* get_arena(void);
Player* find_player_by_name(Player** players, int playerCount, const char* name);
Player* find_player_by_uuid(Player** players, int playerCount, uint32_t uuid);
Fight* find_fight(Fight* fights, int fightCount, uint32_t sessionID);
int register_waiting(uint32_t uuid, uint16_t seq, const char* name);
int register_initiate(uint32_t uuid, uint16_t seq, uint32_t opponentUUID, uint32_t sessionID, int fighter, char moves[4]);
int register_accept(uint32_t uuid, uint16_t seq, uint32_t opponentUUID, uint32_t sessionID, int fighter, char moves[4]);
int register_fled(uint32_t uuid, uint16_t seq, uint32_t sessionID);
int register_move(uint32_t uuid, uint16_t seq, uint32_t sessionID, int i);

#endif //GAME_H
