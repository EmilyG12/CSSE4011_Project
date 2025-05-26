#ifndef VIEWER_H
#define VIEWER_H
#include <stdbool.h>

typedef struct {
    const char* label;
    int id;
    bool on;
    void (*callback)(int id);
} ButtonConfig;

void init_screen(void);

typedef struct {
    ButtonConfig* buttons;
    int buttonCount;
} ConnectionSceneConfig;

ConnectionSceneConfig* init_connections_scene(ConnectionSceneConfig* config);
void update_connections_scene(ConnectionSceneConfig* config);

typedef struct {
    int health;
    int healthMax;
    const char* name;
    int playerNum;
    int turn;
    const char* spriteName;
} PlayerDisplayConfig;

typedef struct {
    PlayerDisplayConfig me;
    PlayerDisplayConfig opponent;
    ButtonConfig* buttons;
    int buttonCount;
} BattleSceneConfig;

BattleSceneConfig* init_battle_scene(BattleSceneConfig* config);
void update_battle_scene(BattleSceneConfig* config);

#endif