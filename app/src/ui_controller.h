#ifndef UI_CONTROLLER_H
#define UI_CONTROLLER_H

#include "game_controller.h"

typedef struct {
    void (*init)(void);
    void (*update)(void);
} ScreenController;

typedef struct {
    ScreenController battle;
    ScreenController waiting;
    void(*buttonPressed)(char letter);
} UiController;

UiController *init_ui(GameController *controller);

#endif // UI_CONTROLLER_H