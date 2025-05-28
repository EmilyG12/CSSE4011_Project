//
// Created by Sioryn Willett on 2025-05-27.
//

#include "pokedex.h"
#include <stdlib.h>
#include <zephyr/kernel.h>

#include <zephyr/shell/shell.h>

Pokemon all_pokemon[] = {
//  { No,   "name",         HP, ATK,    DEF, Sp.ATK, Sp.DEF,    Moves Allowed},
    {  0,	"unknown",       0,	  0,	  0,	  0,	  0,	{}},
    {  1,	"bulbasaur",    45,	 49,	 49,	 65,	 65,	{2, 3, 4, 5, 6, 7, 8}},
    {  4,	"charmander",   39,	 52,	 43,	 60,	 50,	{9, 10, 11, 12, 13, 14, 15, 16, 17}},
    {  7,	"squirtle",     44,	 48,	 65,	 50,	 64,	{2, 18, 19, 20, 21, 22, 23, 24}},
    { 16,	"pidgey",       40,	 45,	 40,	 35,	 35,	{1, 2, 3, 4}},
    { 25,	"pikachu",      35,	 55,	 40,	 50,	 50,	{1, 2, 3, 4}},
    { 35,	"clefairy",     70,	 45,	 48,	 60,	 65,	{1, 2, 3, 4}},
    { 61,	"poliwhirl",    65,	 65,	 65,	 50,	 50,	{1, 2, 3, 4}},
    {151,	"mew",         100,	100,	100,	100,	100,	{1, 2, 3, 4}},
};

Move all_moves[] = {
//  {N, "name",				                   ATK,          Sp.ATK},
    { 0, "nothing",			  0,	  0},
    { 1, "chop",				  1,	  0},
    { 2, "Tackle",			 40,	  0},
    { 3, "Vine Whip",		 45,	  0},
    { 4, "Razor Leaf",		 55,	  0},
    { 5, "Seed Bomb",		 80,	  0},
    { 6, "Take Down",		 90,	  0},
    { 7, "Power Whip",		120,	  0},
    { 8, "Solar Beam",		  0,	120},
    { 9, "Scratch",	    	 40,	  0},
    {10, "Ember",			 40,	  0},
    {11, "Dragon Breath",	  0,	 60},
    {12, "Fire Fang",	     60,	  0},
    {13, "Slash",		     65,	  0},
    {14, "Flamethrower",      0,	 70},
    {15, "Fire Spin",	      0,	 90},
    {16, "Inferno",		      0,	 35},
    {17, "Flare Blitz",	    100,	  0},
    {18, "Water Gun",	      0,	 40},
    {19, "Rapid Spin",	     40,	  0},
    {20, "Bite",		    	 50,	  0},
    {21, "Water Pulse",	      0,	 60},
    {22, "Aqua Tail",	     60,	  0},
    {23, "Hydro Pump",	      0,	 90},
    {24, "Wave Crash",	    110,	  0},
};

Pokemon* get_pokemon(int id){
    for (int i = 0; i < ARRAY_SIZE(all_pokemon); i++){
        if (all_pokemon[i].id == id){
            return &all_pokemon[i];
        }
    }
    return get_pokemon(0);
}

Pokemon* get_pokemon_by_name(const char* name){
    int n = atoi(name);
    if (n) {
        return get_pokemon(n);
    }

    for (int i = 0; i < ARRAY_SIZE(all_pokemon); i++){
        if (!strcmp(all_pokemon[i].name, name)){
            return &all_pokemon[i];
        }
    }

    return get_pokemon(0);
}

Move* get_move(char id){
    for (int i = 0; i < ARRAY_SIZE(all_moves); i++){
        if (all_moves[i].id == id){
            return &all_moves[i];
        }
    }

    return get_move(0);
}

void pokemon_summary(const struct shell* shell, Pokemon* pokemon) {
    shell_print(shell, "%-15s\tATK: %3d\tDEF: %3d\tSp. ATK: %3d\tSp. DEF: %3d", pokemon->name,
                pokemon->power, pokemon->defense, pokemon->specialPower, pokemon->specialDefense);
}

void pokemon_details(const struct shell* shell, Pokemon* pokemon) {
    shell_print(shell, "%s", pokemon->name);
    shell_print(shell, "ATK: %d\tDEF: %d\tSp. ATK: %d\tSp. DEF: %d",
                pokemon->power, pokemon->defense, pokemon->specialPower, pokemon->specialDefense);
    for (int i = 0; i < 10; i++){
        if (pokemon->legalMoves[i]){
            Move* move = get_move(pokemon->legalMoves[i]);
            shell_print(shell, "%13s: ATK=%3d\tSp.ATK=%3d", move->name, move->power, move->specialPower);
        }
    }
}

int list_pokemon(const struct shell* shell){
    for (int i = 0; i < ARRAY_SIZE(all_pokemon); i++){
        pokemon_summary(shell, &all_pokemon[i]);
    }
    return 0;
}

int cmd_pokedex(const struct shell* shell, size_t argc, char** argv){
    if (argc == 1){
        return list_pokemon(shell);
    }

    Pokemon* pokemon = get_pokemon_by_name(argv[1]);
    pokemon_details(shell, pokemon);
    return 0;
}

SHELL_CMD_REGISTER(pokedex, NULL, "Read data in the pokedex", cmd_pokedex);
