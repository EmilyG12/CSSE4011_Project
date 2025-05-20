//
// Created by Sioryn Willett on 2025-05-14.
//

#ifndef FIGHT_AD_H
#define FIGHT_AD_H
#include <stddef.h>
#include <stdint.h>

typedef enum {
    FC_WAITING  = 0x00,
    FC_INITIATE = 0x01,
    FC_ACCEPT   = 0x02,
    FC_MOVE_0   = 0x10,
    FC_MOVE_1,
    FC_MOVE_2,
    FC_MOVE_3,
    FC_FLEE     = 0xFF,
    FC_DONE     = 0xD0
} FightCommand;

typedef struct FightAd {
    uint32_t* uuid;
    uint32_t* sessionID;
    uint16_t* sequenceNumber;
    uint16_t* command;
    uint8_t*  args;
} FightAd;

int init_fight_bt(void);

int fight_ad_waiting(const char* name);
int fight_ad_initiate(uint32_t opponentUUID, uint16_t fighter, uint8_t moves[]);
int fight_ad_accept(uint32_t opponentUUID, uint32_t sessionID, uint16_t fighter, uint8_t moves[]);
int fight_ad_decline(uint32_t sessionID, uint32_t opponentUUID);
int fight_ad_move(int move);
int fight_ad_flee(void);

FightAd parse_fight_ad(uint8_t data[], size_t len);

FightAd get_fight_ad(void);

#endif //FIGHT_AD_H
