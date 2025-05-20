//
// Created by Sioryn Willett on 2025-05-18.
//

#include "fight_observer.h"

#include <zephyr/bluetooth/gap.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <zephyr/shell/shell.h>

#include "fight_ad.h"
#include "fight.h"
#include "game_controller.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(fight);

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
    if (!known) {
        arena.waitingPlayers = realloc(arena.waitingPlayers, (arena.waitingCount + 2) * sizeof(Player));
        arena.waitingPlayers[arena.waitingCount] = (Player){
            .uuid = *ad.uuid,
            .sequenceNumber = *ad.sequenceNumber - 1,
        };
        strncpy(arena.waitingPlayers[arena.waitingCount++].name, (char*) ad.args, 16);
        known = find_player(arena.waitingPlayers, arena.waitingCount, *ad.uuid);
    }


    if (known->sequenceNumber != *ad.sequenceNumber) {
        LOG_INF("[%s (%d->%d)]: uuid: 0x%x, session: 0x%x, Waiting to fight!",
            known->name, known->sequenceNumber, *ad.sequenceNumber, known->uuid, *ad.sessionID);
        known->sequenceNumber = *ad.sequenceNumber;
    }
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

    known->fighter = *(uint16_t*) (ad.args + sizeof(uint32_t));
    memcpy(ad.args + sizeof(uint32_t) + sizeof(uint16_t), known->moves, 4);

    if (known->sequenceNumber != *ad.sequenceNumber) {
        LOG_INF("[%s (%d->%d)]: Initiated a duel with",
            known->name, known->sequenceNumber, *ad.sequenceNumber, known->uuid, *ad.sessionID);
        known->sequenceNumber = *ad.sequenceNumber;
    }
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
    for (uint8_t i = 0; i < len; i++) {
        uint8_t adLen = data[i++];
        if (adLen == 0) {
            break;
        }

        uint8_t data_type = data[i++];

        if (data_type == BT_DATA_MANUFACTURER_DATA) {
            FightAd fight_ad = parse_fight_ad(data + i, adLen);
            if (!fight_ad.uuid) {
                continue;
            }
            register_ad(fight_ad);
            return true;
        }
    }
    return false;
}

int cmd_list(const struct shell* shell, int argc, char *argv[]) {
    if (!arena.waitingCount) {
        shell_print(shell, "No players waiting");
        return 0;
    }

    for (int i = 0; i < arena.waitingCount; i++) {
        shell_print(shell, "%s: %x", arena.waitingPlayers[i].name, arena.waitingPlayers[i].uuid);
    }
    return 0;
}

SHELL_CMD_ARG_REGISTER(list, NULL, "List fighters waiting", cmd_list, 1, 0);