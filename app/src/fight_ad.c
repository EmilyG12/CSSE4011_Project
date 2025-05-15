//
// Created by Sioryn Willett on 2025-05-14.
//

#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <bt/bluetooth.h>
#include <zephyr/sys/util.h>

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

uint8_t fightAdData[] = {
    0xFF, 0xFF, // Vendor (Custom)
    0xDE, 0xAD, // Device Type (Fight game)
    // Our UUID
    0xe1, 0xfd, 0x11, 0xdc,
    // Session ID
    0x00, 0x00, 0x00, 0x00,
    // Sequence Number
    0x00, 0x00,
    // Command
    FC_WAITING, FC_WAITING,
    // Args
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

struct FightAd {
    uint32_t* const ourUUID;
    uint32_t* const sessionID;
    uint16_t* const sequenceNumber;
    uint16_t* const command;
    uint8_t*  const args;
} fightAd = {
    (void*) (fightAdData + 4),
    (void*) (fightAdData + 8),
    (void*) (fightAdData + 16),
    (void*) (fightAdData + 18),
    (fightAdData + 20)
};

int init_fight_bt(void){
    if (init_bt()) {
        return 1;
    }

    if (register_advertisement((Advertisement){
        BT_DATA_MANUFACTURER_DATA,
        fightAdData,
        ARRAY_SIZE(fightAdData)
    })) {
        return 2;
    }

    return 0;
}

int fight_ad_initiate(uint32_t opponentUUID, uint16_t fighter, uint8_t moves[]){
    (*fightAd.sessionID) = rand();
    (*fightAd.sequenceNumber)++;
    (*fightAd.command) = FC_INITIATE;

    int offset = 0;
    memcpy(fightAd.args + offset, &opponentUUID, sizeof(opponentUUID));
    offset += sizeof(opponentUUID);

    memcpy(fightAd.args + offset, &fighter, sizeof(fighter));
    offset += sizeof(fighter);

    memcpy(fightAd.args + offset, moves, sizeof(moves[0]) * 4);
    // offset += sizeof(moves[0]) * 4;

    return !update_advertisements();
}

int fight_ad_accept(uint32_t opponentUUID, uint32_t sessionID, uint16_t fighter, uint8_t moves[]){
    (*fightAd.sessionID) = sessionID;
    (*fightAd.sequenceNumber)++;
    (*fightAd.command) = FC_ACCEPT;

    int offset = 0;
    memcpy(fightAd.args + offset, &opponentUUID, sizeof(opponentUUID));
    offset += sizeof(opponentUUID);

    memcpy(fightAd.args + offset, &fighter, sizeof(fighter));
    offset += sizeof(fighter);

    memcpy(fightAd.args + offset, moves, sizeof(moves[0]) * 4);
    // offset += sizeof(moves[0]) * 4;

    return !update_advertisements();
}

int fight_ad_decline(uint32_t sessionID, uint32_t opponentUUID){
    (*fightAd.sessionID) = sessionID;
    (*fightAd.sequenceNumber)++;
    (*fightAd.command) = FC_WAITING;
    memcpy(fightAd.args, &opponentUUID, sizeof(opponentUUID));

    return !update_advertisements();
}

int fight_ad_move(int move){
    (*fightAd.sequenceNumber)++;
    (*fightAd.command) = FC_MOVE_0 + move;
    return !update_advertisements();
}

int fight_ad_flee(int move){
    (*fightAd.sequenceNumber)++;
    (*fightAd.command) = FC_FLEE;

    return !update_advertisements();
}
