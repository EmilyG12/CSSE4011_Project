//
// Created by Sioryn Willett on 2025-05-27.
//

#include "user.h"
#include "pokedex.h"
#include <stdlib.h>
#include <string.h>

#include <zephyr/shell/shell.h>

User user_data = {
    .fighter = {.id = 1, .moves = {1, 1, 1, 1}}
};

User* get_user(void){
    return &user_data;
}

void set_user_name(const char* name){
  strncpy(user_data.name, name, 16);
}

void set_user_fighter(int fighter, char moves[4]){
    user_data.fighter.id = fighter;
    for (int i = 0; i < 4; i++){
      user_data.fighter.moves[i] = moves[i];
    }
}

int cmd_user_name(const struct shell* shell, int argc, char* argv[]){
    if (argc > 1) {
        strcpy(user_data.name, argv[1]);
    }

    shell_print(shell, "Your name is: %s", user_data.name);
    return 0;
}

int cmd_user_fighter(const struct shell* shell, int argc, char* argv[]){
    if (argc > 1) {
        Pokemon* pokemon = get_pokemon_by_name(argv[1]);
        user_data.fighter.id = pokemon->id;

        for (int i = 0; i < 4; i++){
            user_data.fighter.moves[i] = (argc > 2 + i) ? strtol(argv[2 + i], NULL, 10) : pokemon->legalMoves[i];
        }
    }

    shell_print(shell, "%s:", get_pokemon(user_data.fighter.id)->name);
    for (int i = 0; i < 4; i++){
        Move* m = get_move(user_data.fighter.moves[i]);
        shell_print(shell, "%s: %d %d", m->name, m->power, m->specialPower);
    }
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_user,
        SHELL_CMD(name,    NULL, "Set or display your name", cmd_user_name),
        SHELL_CMD(fighter, NULL, "Set or display your chosen fighter", cmd_user_fighter),
        SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(user, &sub_user, "User account commands", NULL);