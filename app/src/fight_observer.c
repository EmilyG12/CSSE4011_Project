//
// Created by Sioryn Willett on 2025-05-18.
//

#include "fight_observer.h"

#include <zephyr/bluetooth/gap.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "fight_ad.h"
#include "fight.h"
#include "game_controller.h"

typedef struct {
    int waitingCount;
    Player* waitingPlayers;
    int pendingCount;
    Player* pendingPlayers;
    int fightCount;
    Fight* fights;
} Arena;

Arena arena;

Player* find_player(Player* players, int playerCount, uint32_t uuid) {
    for (int i = 0; i < playerCount; i++) {
        if (players[i].uuid == uuid) {
            return players + i;
        }
    }
    return NULL;
}

void register_waiting(FightAd ad) {
    Player* known = find_player(arena.waitingPlayers, arena.waitingCount, *ad.uuid);
    if (known) {
        known->sequenceNumber = *ad.sequenceNumber;
        return;
    }

    arena.waitingPlayers = realloc(arena.waitingPlayers, (arena.waitingCount + 2) * sizeof(Player));
    arena.waitingPlayers[arena.waitingCount] = (Player){
        .uuid = *ad.uuid,
        .sequenceNumber = *ad.sequenceNumber,
    };
    strncpy(arena.waitingPlayers[arena.waitingCount++].name, (char*) ad.args, 16);
}

Player remove_player_or_create(Player* players, int* playerCount, uint32_t uuid) {
    for (int i = 0; i < *playerCount; i++) {
        if (players[i].uuid == uuid) {
            Player found = players[i];
            players[i] = players[*playerCount - 1];
            *playerCount -= 1;
            return found;
        }
    }

    Player player = {.uuid = uuid};
    strcpy(player.name, "unknown");
    return player;
}

void register_initiate(FightAd ad) {
    Player* known = find_player(arena.pendingPlayers, arena.pendingCount, *ad.uuid);
    if (!known) {
        Player player = remove_player_or_create(arena.waitingPlayers, &arena.waitingCount, *ad.uuid);
        player.sequenceNumber = *ad.sequenceNumber;
        arena.pendingPlayers = realloc(arena.pendingPlayers, (arena.pendingCount + 2) * sizeof(Player));
        arena.pendingPlayers[arena.pendingCount] = player;
        known = arena.pendingPlayers + arena.waitingCount++;
    }

    known->sequenceNumber = *ad.sequenceNumber;
    known->fighter = *(uint16_t*) (ad.args + sizeof(uint32_t));
    memcpy(ad.args + sizeof(uint32_t) + sizeof(uint16_t), known->moves, 4);
}

void register_ad(FightAd ad) {
    switch (*ad.command) {
        case FC_WAITING:
            register_waiting(ad);
            return;
        case FC_INITIATE:
            register_initiate(ad);
            return;
        case FC_ACCEPT:
            // register_accept(ad);
        default:

    }
}


bool fight_ad_observe_arena(const char *mac_addr, int rssi, int type, uint8_t data[], size_t len) {
    if (type != BT_DATA_MANUFACTURER_DATA) {
        return false;
    }

    FightAd fight_ad = parse_fight_ad(data, len);
    if (!fight_ad.uuid) {
        return false;
    }

    register_ad(fight_ad);

    return true;
}


