//
// Created by Sioryn Willett on 2025-05-27.
//

#ifndef POKEDEX_H
#define POKEDEX_H

typedef struct {
  int id;
  const char* name;
  int maxHP;
  int power;
  int defense;
  int specialPower;
  int specialDefense;
  char legalMoves[10];
} Pokemon;

typedef struct {
    char id;
    const char* name;
    int power;
    int specialPower;
} Move;

Pokemon* get_pokemon(int id);

Move* get_move(char id);

#endif //POKEDEX_H
