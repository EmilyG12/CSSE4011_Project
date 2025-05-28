//
// Created by Sioryn Willett on 2025-05-27.
//

#ifndef EVENT_H
#define EVENT_H

typedef struct {
    int (*callbacks[5])(void*);
    int callbackCount;
    int (*trigger)(void*);
    int (*register_cb)(int(*func)(void*));
} Event;

typedef struct {
  int a;
} Callbacks;

#endif //EVENT_H
