//
// Created by Sioryn Willett on 2025-05-19.
//

#include "game.h"
#include "pokedex.h"
#include "fight.h"

#include <zephyr/bluetooth/gap.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <zephyr/shell/shell.h>
#include <zephyr/kernel.h>

#include "fight_ad.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(fight);

static Arena arena = {};

Player* find_player_by_name(Player** players, int playerCount, const char* name) {
    long uuid = strtol(name, NULL, 0);
    if (uuid) {
        return find_player_by_uuid(players, playerCount, uuid);
    }

    for (int i = 0; i < playerCount; i++) {
        if (!strcmp(players[i]->name, name)) {
            return players[i];
        }
    }

    return NULL;
}

Player* find_player_by_uuid(Player** players, int playerCount, uint32_t uuid) {
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

    Player* known = find_player_by_uuid(arena.waitingPlayers, arena.waitingCount, uuid);
    if (!known) {
        add_player(arena.waitingPlayers, &arena.waitingCount, player);
        known = find_player_by_uuid(arena.waitingPlayers, arena.waitingCount, uuid);
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

    Player* known = find_player_by_uuid(arena.pendingPlayers, arena.pendingCount, uuid);
    if (!known) {
        remove_player(arena.waitingPlayers, &arena.waitingCount, player);
        add_player(arena.pendingPlayers, &arena.pendingCount, player);
        known = find_player_by_uuid(arena.pendingPlayers, arena.pendingCount, uuid);
    }

    if (!known) {
        LOG_ERR("new player couldn't be added to the pending list");
        return -1;
    }

    known->challengee = opponentUUID;
    known->sessionID = sessionID;
    known->fighter = fighter;
    known->hp = get_pokemon(fighter)->maxHP;
    memcpy(known->moves, moves, 4);

    Player* challengee = find_player_by_uuid(arena.waitingPlayers, arena.waitingCount, opponentUUID);
    char* name = challengee ? challengee->name : "<unknown>";

    LOG_INF("[%s (%d->%d)]: Initiated a duel against [%s]",
        known->name, lastSeq, known->sequenceNumber, name);
    return 1;
}

int register_accept(uint32_t uuid, uint16_t seq, uint32_t opponentUUID, uint32_t sessionID, int fighter, char moves[4]) {
    LOG_DBG("register_accept(uuid=0x%x, seq=%d, opponentUUID=0x%x, sessionID=0x%x, fighter=%d, moves=[%d, %d, %d, %d])",
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

        Player* challenger = find_player_by_uuid(arena.pendingPlayers, arena.pendingCount, opponentUUID);
        if (!challenger || (challenger->challengee != uuid)) {
            LOG_ERR("[%s: 0x%x] accepted invalid challenge", player->name, player->uuid);
            player->sequenceNumber = seq;
            return -1;
        }

        remove_player(arena.pendingPlayers, &arena.pendingCount, challenger);
        remove_player(arena.waitingPlayers, &arena.waitingCount, player);

        player->fighter = fighter;
        player->hp = get_pokemon(fighter)->maxHP;
        memcpy(player->moves, moves, 4);
        player->sessionID = sessionID;

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

int calculate_damage(Move* move, Pokemon* attacker, Pokemon* defender) {
    int physicalDamage = (move->power * attacker->power) / defender->defense;
    int specialDamage = (move->specialPower * attacker->specialPower) / defender->specialDefense;
    return physicalDamage + specialDamage;
}

void finalise_fight(Fight* fight, Player* winner) {
    Player* challenger_cpy = malloc(sizeof(Player));
    *challenger_cpy = *fight->players[0];

    Player* challengee_cpy = malloc(sizeof(Player));
    *challengee_cpy = *fight->players[1];

    fight->players[0] = challenger_cpy;
    fight->players[1] = challengee_cpy;
    fight->winner = (winner->uuid == fight->players[0]->uuid) ? challenger_cpy : challengee_cpy;
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

    Player* known = find_player_by_uuid(fight->players, 2, uuid);
    if (!known) {
        LOG_ERR("[%s: 0x%x] fought in a fight they weren't in", player->name, player->uuid);
        player->sequenceNumber = seq;
        return -3;
    }

    if (move < 0 || move > 3) {
        LOG_ERR("[%s: 0x%x] invalid move number %d", player->name, player->uuid, move);
        return -4;
    }

    Move* m = get_move(known->moves[move]);
    if (!m->id) {
        LOG_ERR("[%s: 0x%x] invalid move %d", player->name, player->uuid, known->moves[move]);
        return -5;
    }

    fight->moves[fight->moveCount++] = move;
    Player* opponent = (fight->players[0]->uuid == known->uuid) ? fight->players[1] : fight->players[0];
    int damage = calculate_damage(m, get_pokemon(known->fighter), get_pokemon(opponent->fighter));
    opponent->hp -= damage;

    if (opponent->hp <= 0) {
        finalise_fight(fight, player);
    }

    LOG_INF("[%s: 0x%x] Performed a %s against [%s: 0x%x] dealing %d damage",
        known->name, known->uuid, m->name, opponent->name, opponent->uuid, damage);
    return 1;
}

int register_fled(uint32_t uuid, uint16_t seq, uint32_t sessionID) {
    LOG_DBG("register_fled(uuid=0x%x, seq=%d, sessionID=0x%x)",
                uuid, seq, sessionID);
    Fight* fight = find_fight(arena.fights, arena.fightCount, sessionID);
    if (!fight) {
        LOG_ERR("Fight not recognised");
        return -1;
    }

    Player* player = find_player_by_uuid(fight->players, 2, uuid);
    if (!player) {
        LOG_ERR("Player not a member of that fight");
        return -2;
    }

    if (player->sequenceNumber == seq) {
        return 0;
    }
    player->sequenceNumber = seq;

    finalise_fight(fight, player->uuid == fight->players[0]->uuid
            ? fight->players[1]
            : fight->players[0]);

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
        Player* challengee = find_player_by_uuid(arena.waitingPlayers, arena.waitingCount, challenger->challengee);

        shell_print(shell, "[%s: 0x%x] challenges [%s: 0x%x]\nwith the fighter (%s){%s, %s, %s, %s}",
            challenger->name,
            challenger->uuid,
            challengee ? challengee->name : "<unknown>",
            challenger->challengee,
            get_pokemon(challenger->fighter)->name,
            get_move(challenger->moves[0])->name,
            get_move(challenger->moves[1])->name,
            get_move(challenger->moves[2])->name,
            get_move(challenger->moves[3])->name);
    }
    shell_print(shell, "");
}

void print_fighter(const struct shell* shell, Player* player) {
    Pokemon* p = get_pokemon(player->fighter);
    shell_print(shell, "[%s: 0x%x] with fighter %s (%d/%d){%s, %s, %s, %s}",
            player->name, player->uuid,
            p->name,
            player->hp,
            p->maxHP,
            get_move(player->moves[0])->name,
            get_move(player->moves[1])->name,
            get_move(player->moves[2])->name,
            get_move(player->moves[3])->name);
}

void print_fight(const struct shell* shell, Fight* fight) {
    Player* challenger = fight->players[0];
    Player* challengee = fight->players[1];

    print_fighter(shell, challenger);
    const char* state = (!fight->winner) ? "VS" :
            (fight->winner == challenger) ?
                    "defeated" : "lost to";
    shell_print(shell, "%s", state);
    print_fighter(shell, challengee);
    for (int m = 0; m < fight->moveCount; m++) {
        int fighter_move_number = fight->moves[m];
        char global_move_number = fight->players[m % 2]->moves[fighter_move_number];
        Move* move = get_move(global_move_number);

        shell_fprintf_normal(shell, "%s%s", move->name, m % 2 ? " - " : "|");
    }
    shell_print(shell, "");
}

void print_fights(const struct shell* shell) {
    if (!arena.fightCount) {
        shell_print(shell, "No ongoing fights");
        return;
    }

    shell_print(shell, "Ongoing fights:");
    for (int i = 0; i < arena.fightCount; i++) {
        Fight* fight = arena.fights + i;
        if (!fight->winner) {
            print_fight(shell, fight);
        }
    }

    shell_print(shell, "Finished fights:");
    for (int i = 0; i < arena.fightCount; i++) {
        Fight* fight = arena.fights + i;
        if (fight->winner) {
            print_fight(shell, fight);
        }
    }
}

int cmd_arena(const struct shell* shell, int argc, char *argv[]) {
    print_waiting(shell);
    print_challenges(shell);
    print_fights(shell);
    return 0;
}

SHELL_CMD_REGISTER(arena, NULL, "See the current state of tracked fights and nearby players", cmd_arena);
