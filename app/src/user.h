//
// Created by Sioryn Willett on 2025-05-27.
//

#ifndef USER_H
#define USER_H

typedef struct {
    int id;
    char moves[4];
} Fighter;

typedef struct {
    char name[16];
    Fighter fighter;
} User;

User* get_user(void);

void set_user_name(const char* name);
void set_user_fighter(int fighter, char moves[4]);

#endif //USER_H
