//
// Created by Sioryn Willett on 2025-05-18.
//

#include "controller.h"

#include <zephyr/bluetooth/gap.h>
#include <zephyr/random/random.h>

#include "fight_ad.h"
#include "game.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(app);

GameController *game_controller = NULL;

typedef struct {
    uint8_t data[32];
    FightAd ad;
} FightAdMessage;

int wait_cmd(const struct shell *shell, int argc, char **argv) {
    if (argc != 1) {
        shell_print(shell, "Usage: fight wait name");
        return 1;
    }

    FightAd my_ad = get_fight_ad();
    return game_controller->me.waiting(*my_ad.uuid, *my_ad.sequenceNumber + 1, argv[0]);
}

int initiate_cmd(const struct shell *shell, int argc, char **argv) {
    if (argc != 6) {
        shell_print(shell, "Usage: fight initiate opponent_uuid fighter_id move_id_1 move_id_2 move_id_3 move_id_4");
        return 1;
    }

    FightAd my_ad = get_fight_ad();
    char moves[] = {atoi(argv[2]), atoi(argv[3]), atoi(argv[4]), atoi(argv[5])};
    return game_controller->me.initiate(*my_ad.uuid,
                                        *my_ad.sequenceNumber + 1,
                                        strtol(argv[0], NULL, 0),
                                        sys_rand32_get(),
                                        atoi(argv[1]),
                                        moves);
}

int accept_cmd(const struct shell *shell, int argc, char **argv) {
    if (argc != 7) {
        shell_print(shell, "Usage: fight accept opponent_uuid session_id fighter_id move_id_1 move_id_2 move_id_3 move_id_4");
        return 1;
    }

    FightAd my_ad = get_fight_ad();
    char moves[] = {atoi(argv[3]), atoi(argv[4]), atoi(argv[5]), atoi(argv[6])};
    return game_controller->me.accept(*my_ad.uuid,
                                      *my_ad.sequenceNumber + 1,
                                      strtol(argv[0], NULL, 0),
                                      strtol(argv[1], NULL, 0),
                                      atoi(argv[2]),
                                      moves);
}

int move_cmd(const struct shell *shell, int argc, char **argv) {
    if (argc != 1) {
        shell_print(shell, "Usage: fight move move_id");
        return 1;
    }

    FightAd my_ad = get_fight_ad();
    return game_controller->me.move(*my_ad.uuid, *my_ad.sequenceNumber + 1, *my_ad.sessionID, atoi(argv[0]));
}

int flee_cmd(const struct shell *shell, int argc, char **argv) {
    if (argc != 0) {
        shell_print(shell, "Usage: fight flee");
        return 1;
    }

    FightAd my_ad = get_fight_ad();
    return game_controller->me.fled(*my_ad.uuid, *my_ad.sequenceNumber + 1, *my_ad.sessionID);
}

int process_cmd(const struct shell *shell, int argc, char **argv) {
    if (!game_controller) {
        shell_print(shell, "Game has not been initialised");
        return 1;
    }

    if (argc < 1) {
        shell_print(shell, "Usage: fight command [args]");
        return 2;
    }

    struct {const char* name; int(*cmd)(const struct shell *shell, int argc, char **argv);} commands[] = {
        {"wait", wait_cmd},
        {"initiate", initiate_cmd},
        {"accept", accept_cmd},
        {"move", move_cmd},
        {"flee", flee_cmd},
    };
    for (int i = 0; i < ARRAY_SIZE(commands); i++) {
        if (!strcmp(argv[0], commands[i].name)) {
            return commands[i].cmd(shell, argc - 1, argv + 1);
        }
    }

    shell_print(shell, "\"%s\" command not recognised", argv[0]);
    return -1;
}

void button_pressed(int i) {
    if (!game_controller) {
        return;
    }

    // TODO
}

void copy_moves(char* out, uint8_t* in) {
    for (int i = 0; i < 4; i++) {
        out[i] = (char) in[i];
    }
}

int fight_ad_process(FightAd ad) {
    PlayerController opponent = game_controller->opponent;
    char moves[4];
    switch (*ad.command) {
        case FC_DONE:
            return 0;
        case FC_WAITING:
            return opponent.waiting(*ad.uuid, *ad.sequenceNumber, (const char *) ad.args);
        case FC_FLEE:
            return opponent.fled(*ad.uuid, *ad.sequenceNumber, *ad.sessionID);
        case FC_INITIATE:
            copy_moves(moves, ad.args + sizeof(uint32_t) + sizeof(uint16_t));
            return opponent.initiate(*ad.uuid,
                                     *ad.sequenceNumber,
                                     *(uint32_t *) ad.args,
                                     *ad.sessionID,
                                     *(uint16_t *) (ad.args + sizeof(uint32_t)),
                                     moves);
        case FC_ACCEPT:
            copy_moves(moves, ad.args + sizeof(uint32_t) + sizeof(uint16_t));
            return opponent.accept(*ad.uuid,
                                   *ad.sequenceNumber,
                                   *(uint32_t *) ad.args,
                                   *ad.sessionID,
                                   *(uint16_t *) (ad.args + sizeof(uint32_t)),
                                   moves);
        case FC_MOVE_0: case FC_MOVE_1: case FC_MOVE_2: case FC_MOVE_3:
            return opponent.move(*ad.uuid, *ad.sequenceNumber, *ad.sessionID, *ad.command - FC_MOVE_0);
        default:
            LOG_ERR("invalid fight ad detected: uuid=0x%x seq=%d session=0x%x cmd=0x%x",
                *ad.uuid, *ad.sequenceNumber, *ad.sessionID, *ad.command);
            return -1;
    }
}

K_QUEUE_DEFINE(bt_queue);

void process_queue(void) {
    while (!k_queue_is_empty(&bt_queue)) {
        FightAdMessage* ad = k_queue_get(&bt_queue, K_MSEC(33));
        fight_ad_process(ad->ad);
        free(ad);
    }
}

bool arena_observer(const char *mac_addr, int rssi, int type, uint8_t data[], size_t len) {
    if (!game_controller) {
        LOG_ERR("Game has not been initialised");
        return false;
    }

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

            Player* player = find_player(game_controller->arena->players, game_controller->arena->playerCount, *fight_ad.uuid);
            if (player && player->sequenceNumber == *fight_ad.sequenceNumber) {
                return true;
            }

            FightAdMessage* q = malloc(sizeof(FightAdMessage));
            memcpy(q->data, data + i, adLen);
            q->ad = parse_fight_ad(q->data, adLen);
            k_queue_append(&bt_queue, q);
            return true;
        }
    }

    return false;
}

InputController init_input_controller(GameController *controller) {
    game_controller = controller;

    k_queue_init(&bt_queue);
    return (InputController){
        .command = process_cmd,
        .buttonPressed = button_pressed,
        .observer = arena_observer
    };
}
