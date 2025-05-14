//
// Created by Sioryn Willett on 2025-04-24.
//

#ifndef BT_SCANNER_H
#define BT_SCANNER_H

#include "bt_observer.h"

typedef struct {
    int (*add)(Observer advertisement);
} Scanner;

Scanner init_scanner(void);

#endif //BT_SCANNER_H
