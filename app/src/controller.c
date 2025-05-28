//
// Created by Sioryn Willett on 2025-05-18.
//

#include "controller.h"

#include <zephyr/bluetooth/gap.h>
#include <zephyr/random/random.h>

#include "fight_ad.h"
#include "game.h"
#include "user.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(app);

GameController *game_controller = NULL;

int wait_cmd(const struct shell *shell, int argc, char **argv) {
    if (argc > 1) {
        LOG_ERR("Usage: fight wait [name]");
        return 1;
    }

    const char* username = argc == 1 ? argv[0] : get_user()->name;
    if (!username) {
        LOG_ERR("Usage: fight wait name");
        return 2;
    }

    set_user_name(username);
    FightAd my_ad = get_fight_ad();

    return game_controller->me.waiting(*my_ad.uuid, *my_ad.sequenceNumber + 1, username);
}

int initiate_cmd(const struct shell *shell, int argc, char **argv) {
    if (!(argc == 1 || argc == 2 || argc == 6)) {
        LOG_ERR("Usage: fight initiate opponent [fighter_id [move_id_1 move_id_2 move_id_3 move_id_4]]");
        return 1;
    }

    Player* opponent = find_player_by_name(get_arena()->players, get_arena()->playerCount, argv[0]);
    if (!opponent) {
        LOG_ERR("Player \"%s\" not found", argv[0]);
        return 2;
    }

    get_user()->fighter.id = (argc > 1) ? atoi(argv[1]) : get_user()->fighter.id;

    if (argc == 6) {
        for (int i = 0; i < 4; i++) {
            get_user()->fighter.moves[i] = atoi(argv[2 + i]);
        }
    }

    FightAd my_ad = get_fight_ad();

    return game_controller->me.initiate(*my_ad.uuid,
                                        *my_ad.sequenceNumber + 1,
                                        opponent->uuid,
                                        sys_rand32_get(),
                                        get_user()->fighter.id,
                                        get_user()->fighter.moves);
}

int accept_cmd(const struct shell *shell, int argc, char **argv) {
    if (!(argc == 1 || argc == 2 || argc == 6)) {
        LOG_ERR("Usage: fight accept opponent [fighter_id [move_id_1 move_id_2 move_id_3 move_id_4]]");
        return 1;
    }

    Player* challenger = find_player_by_name(get_arena()->pendingPlayers, get_arena()->pendingCount, argv[0]);
    if (!challenger) {
        LOG_ERR("Challenger \"%s\" not found.", argv[0]);
        return 2;
    }

    if (challenger->challengee != *get_fight_ad().uuid) {
        LOG_ERR("Challenger \"%s\" is not challenging us.", challenger->name);
        return 3;
    }

    uint32_t sessionID = challenger->sessionID;

    int fighter = (argc > 1) ? atoi(argv[1]) : get_user()->fighter.id;
    get_user()->fighter.id = fighter;

    if (argc == 6) {
        for (int i = 0; i < 4; i++) {
            get_user()->fighter.moves[i] = atoi(argv[2 + i]);
        }
    }

    FightAd my_ad = get_fight_ad();

    return game_controller->me.accept(*my_ad.uuid,
                                        *my_ad.sequenceNumber + 1,
                                        challenger->uuid,
                                        sessionID,
                                        fighter,
                                        get_user()->fighter.moves);
}

int move_cmd(const struct shell *shell, int argc, char **argv) {
    if (argc != 1) {
        LOG_ERR("Usage: fight move move_id");
        return 1;
    }

    FightAd my_ad = get_fight_ad();
    return game_controller->me.move(*my_ad.uuid, *my_ad.sequenceNumber + 1, *my_ad.sessionID, atoi(argv[0]));
}

int flee_cmd(const struct shell *shell, int argc, char **argv) {
    if (argc != 0) {
        LOG_ERR("Usage: fight flee");
        return 1;
    }

    FightAd my_ad = get_fight_ad();
    return game_controller->me.fled(*my_ad.uuid, *my_ad.sequenceNumber + 1, *my_ad.sessionID);
}

int process_cmd(const struct shell *shell, int argc, char **argv) {
    if (!game_controller) {
        LOG_ERR("Game has not been initialised");
        return 1;
    }

    if (argc < 1) {
        LOG_ERR("Usage: fight command [args]");
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

    LOG_ERR("\"%s\" command not recognised", argv[0]);
    return -1;
}

typedef struct {
    const struct shell *shell;
    int argc;
    char** argv;
} CLIMessage;

K_QUEUE_DEFINE(cmd_queue);

int command_observer(const struct shell *shell, int argc, char **argv) {
    CLIMessage* msg = malloc(sizeof(CLIMessage));
    msg->shell = shell;
    msg->argc = argc;
    msg->argv = malloc(sizeof(char*) * argc);
    for (int i = 0; i < argc; i++) {
        msg->argv[i] = malloc(sizeof(char) * strlen(argv[i]) + 1);
        strcpy(msg->argv[i], argv[i]);
    }

    k_queue_append(&cmd_queue, msg);
    return 0;
}

K_QUEUE_DEFINE(button_queue);

void button_pressed(char letter) {
    if (!game_controller) {
        return;
    }

    char* c = malloc(sizeof(char));
    *c = letter;
    k_queue_append(&button_queue, c);
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

typedef struct {
    uint8_t data[32];
    FightAd ad;
} FightAdMessage;

K_QUEUE_DEFINE(bt_queue);

void process_queue(void) {
    while (!k_queue_is_empty(&bt_queue)) {
        FightAdMessage* ad = k_queue_get(&bt_queue, K_MSEC(10));
        if (!ad) {
            return;
        }
        fight_ad_process(ad->ad);
        free(ad);
    }

    while (!k_queue_is_empty(&cmd_queue)) {
        CLIMessage* cmd = k_queue_get(&cmd_queue, K_MSEC(10));
        if (!cmd) {
            return;
        }
        process_cmd(cmd->shell, cmd->argc, cmd->argv);
        for (int i = 0; i < cmd->argc; i++) {
            free(cmd->argv[i]);
        }
        free(cmd->argv);
        free(cmd);
    }

    while (!k_queue_is_empty(&button_queue)) {
        char* c = k_queue_get(&button_queue, K_MSEC(10));
        if (!c) {
            return;
        }

        game_controller->button_pressed(*c);

        free(c);
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

            Player* player = find_player_by_uuid(game_controller->arena->players, game_controller->arena->playerCount, *fight_ad.uuid);
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
    k_queue_init(&cmd_queue);

    return (InputController){
        .command = command_observer,
        .buttonPressed = button_pressed,
        .observer = arena_observer
    };
}
