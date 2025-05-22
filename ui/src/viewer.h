#ifndef VIEWER_H
#define VIEWER_H

#include <zephyr/drivers/display.h>
#include <lvgl.h>
#include <lvgl_mem.h>
#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/input/input.h>
#include <lvgl_input_device.h>

#define WIDTH 295
#define HEIGHT 220
#define REAL_WIDTH 3
#define REAL_HEIGHT 4
#define HALF_STAR_W 15

#define BUTTON_HEIGHT 40
#define BUTTON_POS_1Y 50

#define P2X 170
#define P2Y 30
#define P1X 110
#define P1Y 190

#define USER 1
#define OPPONENT 2

typedef struct {
    lv_obj_t *scr;
    lv_obj_t *user_hlth;
    lv_obj_t *opponent_hlth;
    lv_obj_t *opp_hlthbar;
    lv_obj_t *user_hlthbar;
} Display;

typedef struct {
    lv_obj_t *button;
    lv_obj_t *label;
    int set;
    int id;
    void (*callback)(int id);
} Button; 

typedef struct {
    const char name[32];
    const lv_img_dsc_t *front;
    const lv_img_dsc_t *back;
} Sprite;

typedef struct {
    int health;
    char pName[32];
    int playerNum;
    int maxHealth;
    int turn;
    char actions[4][32];
    lv_obj_t *sprite;
    const lv_img_dsc_t *sprite_img;
} Player;
//DisPlayer
typedef struct {
    char action[20];
    int setting;
} Action;

void create_screen(void);
void change_logo_position(double newX, double newY);
void change_healthbar(Player *player);
void display_player_health(Player *player);
void initialise_player(Player *player, int playerNum, const char *name, int maxHealth, char actions[4][20]);
// void initialise_player(Player *player, int playerNum, const char name[], int maxHealth, char actions[4][20], uint8_t button_mask);
void change_player_name(Player *player, const char *name);
void change_player_health(Player *player, int health);
void change_player_turn(Player *player, int turn);
void change_player_maxHealth(Player *player, int maxHealth);
void set_battle_scene(void);
void change_player_action(Player *player, char *action, int index, Button *button);
// void change_battle_scene(Player *P1, Player *P2);
void change_player_stats(Player *player, const char *name, int health);
void set_button_mode(Button *button, int mode);
void set_user_sprite(Player *player, const lv_img_dsc_t *img);
Sprite set_sprite_struct(const char *name, const lv_img_dsc_t *front,const lv_img_dsc_t *back);
void change_battle_scene(Player *P1, Player *P2, uint8_t button_mask);

#endif