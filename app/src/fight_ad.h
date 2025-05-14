//
// Created by Sioryn Willett on 2025-05-14.
//

#ifndef FIGHT_AD_H
#define FIGHT_AD_H

int init_fight_bt(void);
int fight_ad_initiate(uint32_t opponentUUID, uint16_t fighter, uint8_t moves[]);
int fight_ad_accept(uint32_t opponentUUID, uint32_t sessionID, uint16_t fighter, uint8_t moves[]);
int fight_ad_decline(uint32_t sessionID, uint32_t opponentUUID);
int fight_ad_move(int move);
int fight_ad_flee(int move);

#endif //FIGHT_AD_H
