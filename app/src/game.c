//
// Created by Sioryn Willett on 2025-05-19.
//

#include "game.h"
#include "fight.h"

#include <zephyr/bluetooth/gap.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <zephyr/shell/shell.h>
#include <zephyr/kernel.h>

#include "fight_ad.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(app);

static Arena arena = {};

Player* find_player(Player** players, int playerCount, uint32_t uuid) {
    for (int i = 0; i < playerCount; i++) {
        if (players[i]->uuid == uuid) {
            return players[i];
        }
    }
    return NULL;
}

Player** add_player(Player** players, int* playerCount, Player* player) {
    if (players == NULL || *playerCount >= MAX_PLAYERS) {
        LOG_INF("Failed to allocate memory for players");
        return NULL;
    }

    players[(*playerCount)++] = player;
    return players;
}

Fight* find_fight(Fight* fights, int fightCount, uint32_t sessionID) {
    for (int i = 0; fights && i < fightCount; i++) {
        if (fights[i].sessionID == sessionID) {
            return fights + i;
        }
    }

    return NULL;
}

void remove_player(Player** players, int* playerCount, Player* player) {
    for (int i = 0; i < *playerCount; i++) {
        if (players[i]->uuid == player->uuid) {
            players[i] = players[*playerCount - 1];
            *playerCount -= 1;
            return;
        }
    }
}

Player* find_or_create(uint32_t uuid, uint16_t seq, const char* name) {
    for (int i = 0; i < arena.playerCount; i++) {
        if (arena.players[i]->uuid == uuid) {
            if (name != NULL) {
                strncpy(arena.players[i]->name, name, 16);
            }
            return arena.players[i];
        }
    }

    Player* player = malloc(sizeof(Player));
    if (!player) {
        LOG_ERR("Failed to allocate memory for player");
        return NULL;
    }

    player->uuid = uuid;
    player->sequenceNumber = seq - 1;
    name = name ? name : "<unknown>";
    strncpy(player->name, name, 16);

    add_player(arena.players, &arena.playerCount, player);

    return player;
}

Arena* get_arena(void) {
    return &arena;
}

int register_waiting(uint32_t uuid, uint16_t seq, const char* name) {
    LOG_DBG("register_waiting(uuid=0x%x, seq=%d, name=%s)", uuid, seq, name);

    Player* player = find_or_create(uuid, seq, name);
    if (!player) {
        LOG_ERR("new player couldn't be created");
        return -1;
    }

    if (player->sequenceNumber == seq) {
        return 0;
    }
    uint16_t lastSeq = player->sequenceNumber;
    player->sequenceNumber = seq;

    Player* known = find_player(arena.waitingPlayers, arena.waitingCount, uuid);
    if (!known) {
        add_player(arena.waitingPlayers, &arena.waitingCount, player);
        known = find_player(arena.waitingPlayers, arena.waitingCount, uuid);
    }

    if (!known) {
        LOG_ERR("new player couldn't be added to the wait list");
        return -1;
    }

    LOG_INF("[%s (%d->%d)]: uuid: 0x%x, Waiting to fight!",
        known->name, lastSeq, known->sequenceNumber, known->uuid);
    return 1;
}

int register_initiate(uint32_t uuid, uint16_t seq, uint32_t opponentUUID, uint32_t sessionID, int fighter, char moves[4]) {
    LOG_DBG("register_initiate(uuid=0x%x, seq=%d, opponentUUID=0x%x, sessionID=0x%x, fighter=%d, moves=[%d, %d, %d, %d])",
          uuid, seq, opponentUUID, sessionID, fighter, moves[0], moves[1], moves[2], moves[3]);

    Player* player = find_or_create(uuid, seq, NULL);
    if (!player) {
        LOG_ERR("failed to find or create player");
        return -1;
    }

    if (player->sequenceNumber == seq) {
        return 0;
    }
    uint16_t lastSeq = player->sequenceNumber;
    player->sequenceNumber = seq;

    Player* known = find_player(arena.pendingPlayers, arena.pendingCount, uuid);
    if (!known) {
        remove_player(arena.waitingPlayers, &arena.waitingCount, player);
        add_player(arena.pendingPlayers, &arena.pendingCount, player);
        known = find_player(arena.pendingPlayers, arena.pendingCount, uuid);
    }

    if (!known) {
        LOG_ERR("new player couldn't be added to the pending list");
        return -1;
    }

    known->challengee = opponentUUID;
    known->fighter = fighter;
    memcpy(known->moves, moves, 4);

    LOG_INF("[%s (%d->%d)]: Initiated a duel (uuid: 0x%x)(session: 0x%x)",
        known->name, lastSeq, known->sequenceNumber, known->uuid, sessionID);
    return 1;
}

int register_accept(uint32_t uuid, uint16_t seq, uint32_t opponentUUID, uint32_t sessionID, int fighter, char moves[4]) {
    LOG_INF("register_accept(uuid=0x%x, seq=%d, opponentUUID=0x%x, sessionID=0x%x, fighter=%d, moves=[%d, %d, %d, %d])",
            uuid, seq, opponentUUID, sessionID, fighter, moves[0], moves[1], moves[2], moves[3]);

    Player* player = find_or_create(uuid, seq, NULL);
    if (!player) {
        LOG_ERR("failed to find or create player");
        return -1;
    }

    if (player->sequenceNumber == seq) {
        return 0;
    }
    uint16_t lastSeq = player->sequenceNumber;
    player->sequenceNumber = seq;

    Fight* fight = find_fight(arena.fights, arena.fightCount, sessionID);
    if (!fight) {
        remove_player(arena.waitingPlayers, &arena.waitingCount, player);

        Player* challenger = find_player(arena.pendingPlayers, arena.pendingCount, opponentUUID);
        if (!challenger || (challenger->challengee != uuid)) {
            LOG_ERR("[%s: 0x%x] accepted invalid challenge", player->name, player->uuid);
            player->sequenceNumber = seq;
            return 0;
        }

        remove_player(arena.pendingPlayers, &arena.pendingCount, challenger);
        remove_player(arena.waitingPlayers, &arena.waitingCount, player);

        player->fighter = fighter;
        memcpy(player->moves, moves, 4);

        Fight newFight = {
            .players[0] = challenger,
            .players[1] = player,
            .sessionID = sessionID,
            .moveCount = 0,
        };
        arena.fights[arena.fightCount++] = newFight;
    }

    LOG_INF("[%s (%d->%d)]: Accepted a duel (uuid: 0x%x)(session: 0x%x)",
        player->name, lastSeq, player->sequenceNumber, player->uuid, sessionID);
    return 1;
}

int register_move(uint32_t uuid, uint16_t seq, uint32_t sessionID, int move) {
    LOG_DBG("register_move(uuid=0x%x, seq=%d, sessionID=0x%x, move=%d)",
              uuid, seq, sessionID, move);

    Player* player = find_or_create(uuid, seq, NULL);
    if (!player) {
        LOG_ERR("failed to find or create player");
        return -1;
    }

    if (player->sequenceNumber == seq) {
        return 0;
    }
    player->sequenceNumber = seq;

    Fight* fight = find_fight(arena.fights, arena.fightCount, sessionID);
    if (!fight) {
        LOG_ERR("[%s: 0x%x] fought in an invalid fight", player->name, player->uuid);
        player->sequenceNumber = seq;
        return -2;
    }

    Player* known = find_player(fight->players, 2, uuid);
    if (!known) {
        LOG_ERR("[%s: 0x%x] fought in a fight they weren't in", player->name, player->uuid);
        player->sequenceNumber = seq;
        return -3;
    }

    fight->moves[fight->moveCount++] = move;

    Player* opponent = (fight->players[0]->uuid == known->uuid) ? fight->players[1] : fight->players[0];
    LOG_INF("[%s: 0x%x] Performed a %d move against [%s: 0x%x]",
        known->name, known->uuid, move, opponent->name, opponent->uuid);
    return 1;
}

int register_fled(uint32_t uuid, uint16_t seq, uint32_t sessionID) {
    LOG_DBG("register_fled(uuid=0x%x, seq=%d, sessionID=0x%x)",
                uuid, seq, sessionID);
    Fight* fight = find_fight(arena.fights, arena.fightCount, sessionID);
    if (!fight) {
        return -1;
    }

    Player* player = find_player(fight->players, 2, uuid);
    if (!player) {
        return -2;
    }

    if (player->sequenceNumber == seq) {
        return 0;
    }

    player->sequenceNumber = seq;

    // TODO publish fight to web.
    return 1;
}

void print_waiting(const struct shell* shell) {
    if (!arena.waitingCount) {
        shell_print(shell, "No players waiting");
        return;
    }

    shell_print(shell, "Players waiting:");
    for (int i = 0; i < arena.waitingCount; i++) {
        shell_print(shell, "%s: 0x%x", arena.waitingPlayers[i]->name, arena.waitingPlayers[i]->uuid);
    }
    shell_print(shell, "");
}

void print_challenges(const struct shell* shell) {
    if (!arena.pendingCount) {
        shell_print(shell, "No challenges issued");
        return;
    }

    shell_print(shell, "Challenges issued:");
    for (int i = 0; i < arena.pendingCount; i++) {
        Player* challenger = arena.pendingPlayers[i];
        Player* challengee = find_player(arena.waitingPlayers, arena.waitingCount, challenger->challengee);

        shell_print(shell, "[%s: 0x%x] challenges [%s: 0x%x] with the fighter (%d){%d, %d, %d, %d}",
            challenger->name,
            challenger->uuid,
            challengee ? challengee->name : "<unknown>",
            challenger->challengee,
            challenger->fighter,
            challenger->moves[0], challenger->moves[1], challenger->moves[2], challenger->moves[3]
            );
    }
    shell_print(shell, "");
}

void print_fighter(const struct shell* shell, Player* player) {
    shell_print(shell, "[%s: 0x%x] with fighter (%d){%d, %d, %d, %d}",
            player->name, player->uuid,
            player->fighter, player->moves[0], player->moves[1], player->moves[2], player->moves[3]
            );
}

void print_fights(const struct shell* shell) {
    if (!arena.fightCount) {
        shell_print(shell, "No ongoing fights");
        return;
    }

    shell_print(shell, "Ongoing fights:");
    for (int i = 0; i < arena.fightCount; i++) {
        Player* challenger = arena.fights[i].players[0];
        Player* challengee = arena.fights[i].players[1];

        print_fighter(shell, challenger);
        shell_print(shell, "VS");
        print_fighter(shell, challengee);
        shell_print(shell, "");
    }
}

int cmd_arena(const struct shell* shell, int argc, char *argv[]) {
    print_waiting(shell);
    print_challenges(shell);
    print_fights(shell);
    return 0;
}

SHELL_CMD_REGISTER(arena, NULL, "See the current state of tracked fights and nearby players", cmd_arena);
