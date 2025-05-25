
#ifndef REC_INPUT_H
#define REC_INPUT_H

void bt_ready(int err);

#define X_BIT BIT(0)
#define Y_BIT BIT(1)

extern struct k_event coord_event;

/* Global Vars */
extern double x_coord; 
extern double y_coord;
extern struct k_work_delayable adv_restart_work;
void adv_restart_handler(struct k_work *work);

#endif