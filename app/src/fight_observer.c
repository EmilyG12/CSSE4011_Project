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



Fight* find_fight(Fight* fights, int fightCount, uint32_t sessionID) {
    for (int i = 0; i < fightCount; i++) {
        if (fights[i].sessionID == sessionID) {
            return fights + i;
        }
    }

    return NULL;
}

int register_waiting(FightAd ad) {
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

    return 0;
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

int register_initiate(FightAd ad) {
    Player* known = find_player(arena.pendingPlayers, arena.pendingCount, *ad.uuid);
    if (!known) {
        Player player = remove_player_or_create(arena.waitingPlayers, &arena.waitingCount, *ad.uuid);
        arena.pendingPlayers = realloc(arena.pendingPlayers, (arena.pendingCount + 2) * sizeof(Player));
        arena.pendingPlayers[arena.pendingCount] = player;
        known = arena.pendingPlayers + arena.pendingCount++;
    }

    known->fighter = *(uint16_t*) (ad.args + sizeof(uint32_t));
    memcpy(ad.args + sizeof(uint32_t) + sizeof(uint16_t), known->moves, 4);

    if (known->sequenceNumber != *ad.sequenceNumber) {
        LOG_INF("[%s (%d->%d)]: Initiated a duel (uuid: 0x%x)(session: 0x%x)",
            known->name, known->sequenceNumber, *ad.sequenceNumber, known->uuid, *ad.sessionID);
        known->sequenceNumber = *ad.sequenceNumber;
    }

    return 0;
}

int register_accept(FightAd ad) {
    Fight* fight = find_fight(arena.fights, arena.fightCount, *ad.sessionID);
    if (!fight) {
        Player accepter = remove_player_or_create(arena.waitingPlayers, &arena.waitingCount, *ad.uuid);
        uint32_t sessionID = *ad.sessionID;
        uint32_t opponentUUID = *(uint32_t*) (ad.args);
        accepter.fighter = *(uint16_t*) (ad.args + sizeof(uint32_t));
        memcpy(ad.args + sizeof(uint32_t) + sizeof(uint16_t), accepter.moves, 4);

        // FIXME check the accept matches the initiate
        Player initiater = remove_player_or_create(arena.pendingPlayers, &arena.pendingCount, opponentUUID);
        Fight newFight = {
            .players[0] = initiater,
            .players[1] = accepter,
            .sessionID = *ad.sessionID,
        };
        arena.fights = realloc(arena.fights, (arena.fightCount + 1) * sizeof(Fight));
        arena.fights[arena.fightCount++] = newFight;
    }

    fight = find_fight(arena.fights, arena.fightCount, *ad.sessionID);
    Player* known = fight->players + ((fight->players[0].uuid == *ad.uuid) ? 0 : 1);

    if (known->sequenceNumber != *ad.sequenceNumber) {
        LOG_INF("[%s (%d->%d)]: Accepted a duel (uuid: 0x%x)(session: 0x%x)",
            known->name, known->sequenceNumber, *ad.sequenceNumber, known->uuid, *ad.sessionID);
        known->sequenceNumber = *ad.sequenceNumber;
    }

    return 0;
}

int register_ad(FightAd ad) {
    switch (*ad.command) {
        case FC_WAITING:
            return register_waiting(ad);
        case FC_INITIATE:
            return register_initiate(ad);
        case FC_ACCEPT:
            return register_accept(ad);
        default:
            return -1;
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
            return !register_ad(fight_ad);
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