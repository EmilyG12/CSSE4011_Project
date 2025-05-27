//
// Created by Sioryn Willett on 2025-05-27.
//

#include "user.h"
#include <stdlib.h>
#include <string.h>

User user_data = {
    .fighter = {.id = 120, .moves = {1, 2, 3, 4}}
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