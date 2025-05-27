//
// Created by Sioryn Willett on 2025-05-18.
//

#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "game_controller.h"
#include <zephyr/shell/shell.h>
#include "../../libs/bt/bt_observer.h"

typedef struct {
    int (*command)(const struct shell* shell, int argc, char** argv);
    void (*buttonPressed)(int id);
    ObserverCallback observer;
} InputController;

InputController init_input_controller(GameController* controller);

void process_queue(void);

#endif //CONTROLLER_H