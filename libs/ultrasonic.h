//
// Created by Sioryn Willett on 2025-05-05.
//

#ifndef ULTRASONIC_H
#define ULTRASONIC_H
#include <stdint.h>

int init_ultrasonic(void(*callback)(uint8_t metres, uint8_t cms));
uint8_t get_ultrasonic_metres(void);
uint8_t get_ultrasonic_cms(void);

#endif //ULTRASONIC_H
