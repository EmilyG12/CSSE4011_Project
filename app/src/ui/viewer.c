#include "viewer.h"
#include <fight.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(viewer);

#ifdef CONFIG_LVGL
#include <zephyr/kernel.h>
#include <zephyr/drivers/display.h>
#include <lvgl.h>
#include <lvgl_mem.h>

#define POKEMON_COUNT 8

#define WIDTH 295
#define HEIGHT 220
#define REAL_WIDTH 3
#define REAL_HEIGHT 4
#define HALF_STAR_W 15

#define BUTTON_HEIGHT 50
#define BUTTON_POS_1Y 20

#define P2X 170
#define P2Y 30
#define P1X 110
#define P1Y 190

#define USER 1
#define OPPONENT 2

LV_IMG_DECLARE(battleground_b);

LV_IMG_DECLARE(bulbafront);
LV_IMG_DECLARE(bulbaback);
LV_IMG_DECLARE(charback);
LV_IMG_DECLARE(charfront);
LV_IMG_DECLARE(squirtback);
LV_IMG_DECLARE(squirtfront);
LV_IMG_DECLARE(charfront);
LV_IMG_DECLARE(charback);
LV_IMG_DECLARE(pidgeback);
LV_IMG_DECLARE(pidgefront);
LV_IMG_DECLARE(pikafront);
LV_IMG_DECLARE(pikaback);
LV_IMG_DECLARE(poliback);
LV_IMG_DECLARE(polifront);
LV_IMG_DECLARE(cleffront);
LV_IMG_DECLARE(clefback);
LV_IMG_DECLARE(mewfront);
LV_IMG_DECLARE(mewback);

typedef struct {
   const char name[32];
   const lv_img_dsc_t *front;
   const lv_img_dsc_t *back;
} Sprite;

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
   int health;
   char pName[32];
   int playerNum;
   int maxHealth;
   int turn;
   lv_obj_t *sprite;
   const lv_img_dsc_t *sprite_img;
} DisPlayer;


Sprite sprites[POKEMON_COUNT] = {
   {"charmander", &charfront, &charback},
   {"squirtle", &squirtfront, &squirtback},
   {"bulbasaur", &bulbafront, &bulbaback},
   {"pidgey", &pidgefront, &pidgeback},
   {"pikachu", &pikafront, &pikaback},
   {"clefairy", &cleffront, &clefback},
   {"mew", &mewfront, &mewback},
   {"poliwhirl", &polifront, &poliback}
};

Sprite* find_sprite(const char* name) {
   for (int i = 0; i < POKEMON_COUNT; i++) {
      if (!strcmp(name, sprites[i].name)) {
         return sprites + i;
      }
   }

   return NULL;
}

Display display;
DisPlayer user;
DisPlayer opponent;

Button buttons[9] = {};
int buttonCount = 0;

char generic_actions[4][20] = {"Attack", "Defend", "Heal", "Flee"};
char generic_actions_2[4][20] = {"Condone", "Chide", "Murder", "Flee"}; 

int scene = 0;

/*
Basic logic on what should be called:
- change_player_health() -- changes health AND displays it (both healthbar and label)
- change_player_name() -- changes name and displays it
- change_player_turn() -- changes turn (should blank out buttons)
- change_player_actions() -- changes one attack and displays it
- set_button_mode() -- changes button state, colours it grey if off, blue otherwise
- change_battle_scene() -- uses player settings to set the scene (pretty much the only thing you'll call)
b1, b2, b3, b4 are battle scene buttons
TODO: a1-10 will be connection buttons (lists top 10 connections, names them, and upon pressing changes ad for m5)
*/

void change_healthbar(DisPlayer *player) {
   int health_set = 100;
   if (player->health != player->maxHealth) {
      health_set = (player->maxHealth > 0) ? (player->health * 100) / player->maxHealth : 0;
   } 
   if (player->playerNum == OPPONENT) {
         lv_bar_set_value(display.opp_hlthbar, health_set, LV_ANIM_ON);
   } else if (player->playerNum == USER) {
         lv_bar_set_value(display.user_hlthbar, health_set, LV_ANIM_ON);
   }
}

void display_player_health(DisPlayer *player) {
   char buffer[100];
   snprintf(buffer, ARRAY_SIZE(buffer), "%s:  %d/%d", player->pName, player->health, player->maxHealth);
   
   if (player->playerNum == OPPONENT) {
      lv_label_set_text(display.opponent_hlth, buffer);
      lv_obj_align(display.opponent_hlth, LV_ALIGN_DEFAULT, P2X, P2Y+10);
   } else if(player->playerNum == USER) {
      lv_label_set_text(display.user_hlth, buffer);
      lv_obj_align(display.user_hlth, LV_ALIGN_DEFAULT, P1X, P1Y+10);
   }
}

void set_user_sprite(DisPlayer *player, const lv_img_dsc_t * img) {
   if (player->sprite) {
      lv_obj_del(player->sprite);
      player->sprite = NULL;
   }
   player->sprite = lv_img_create(display.scr);
   lv_img_set_src(player->sprite, player->sprite_img);
   if (player->playerNum == USER) {
      lv_obj_align(player->sprite, LV_ALIGN_DEFAULT, P1X, P1Y - 76);
   } else if (player->playerNum == OPPONENT) {
      lv_obj_align(player->sprite, LV_ALIGN_DEFAULT, P2X, P2Y);
   }
}

void set_player_sprite(DisPlayer *player, PlayerDisplayConfig* config) {
   Sprite *sprite = find_sprite(config->spriteName);
   if (sprite) {
      player->sprite_img = player->playerNum == USER ? sprite->back : sprite->front;
      set_user_sprite(player, player->sprite_img);
   } else {
      set_user_sprite(player, &bulbaback);
      LOG_ERR("Sprite not found");
   }
}

void initialise_player(DisPlayer *player, PlayerDisplayConfig *config) {
   player->playerNum = config->playerNum;
   strncpy(player->pName, config->name, strlen(config->name));
   player->maxHealth = config->healthMax;
   player->health = config->health;
   player->sprite = NULL;
   player->turn = 1;
   set_player_sprite(player, config);
}

void change_player_health(DisPlayer *player, int health) {
   player->health = health;
   if(player->playerNum == USER) {
      change_healthbar(player);
   } else if (player->playerNum == OPPONENT) {
      change_healthbar(player);
   }
   display_player_health(player);
}

void change_player_name(DisPlayer *player, const char *name) {
   snprintf(player->pName, sizeof(player->pName), "%s", name);
   change_player_health(player, player->health);
}

void change_player_stats(DisPlayer *player, const char *name, int health) {
   snprintf(player->pName, sizeof(player->pName), "%s", name); 
   player->health = health;
   if(player->playerNum == USER) {
      change_healthbar(player);
   } else if (player->playerNum == OPPONENT) {
      change_healthbar(player);
   }
   display_player_health(player);
}

void change_player_turn(DisPlayer *player, int turn) {
   player->turn = turn;
}

void change_player_maxHealth(DisPlayer *player, int maxHealth) {
   player->maxHealth = maxHealth;
}

void change_button_label(Button *button, const char *label) {
#ifndef CONFIG_LVGL
   LOG_ERR("screen not available");
#else
   lv_label_set_text(button->label, label);
#endif
}

void set_button_mode(Button *button, int mode) {
#ifndef CONFIG_LVGL
   LOG_ERR("screen not available");
#else
   button->set = mode;
   if (!mode) {
      lv_obj_set_style_bg_color(button->button, lv_palette_main(LV_PALETTE_GREY), 0);
   } else if (mode == 1){
      lv_obj_set_style_bg_color(button->button, lv_palette_main(LV_PALETTE_BLUE), 0); 
   } else if (mode == 2) {
      lv_obj_set_style_bg_color(button->button, lv_palette_main(LV_PALETTE_PINK), 0); 
   }
#endif
}

void change_player_action(const char *action, int index, Button *button) {
   if (index < 4) {
      change_button_label(button, action);
   } else {
      LOG_ERR("Index out of bounds");
   }

}

lv_obj_t* set_healthbar(DisPlayer *player){
   lv_obj_t *bar = lv_bar_create(lv_scr_act());
   if (player->playerNum == OPPONENT) {
      lv_obj_align(bar, LV_ALIGN_DEFAULT, P2X, P2Y); 
      lv_obj_set_size(bar, 100, 10);
      lv_bar_set_value(bar, 100, LV_ANIM_ON);
      return bar;
   } else if (player->playerNum == USER) {
      lv_obj_align(bar, LV_ALIGN_DEFAULT, P1X, P1Y); 
      lv_obj_set_size(bar, 100, 10);
      lv_bar_set_value(bar, 100, LV_ANIM_ON);
      return bar;
   } else {
      LOG_ERR("Invalid player number");
      return NULL;
   }
}

void redirect_cb(lv_event_t *e) {
   LOG_INF("button pressed!");
   Button *btn = (Button *)lv_event_get_user_data(e);
   if (btn->set) {
      btn->callback(btn->id);
   }
}

Button create_button(void (*callback)(int), lv_obj_t* screen, int x, int y, const char* label, int width, int height, int id, Button *button_ptr) {
      lv_obj_t *btn = lv_obj_create(screen);
      lv_obj_t *txt = lv_label_create(btn);
      lv_obj_align(btn, LV_ALIGN_DEFAULT, x, y);
      lv_label_set_text(txt, label);
      lv_obj_align(txt, LV_ALIGN_CENTER, 0, 0);
      lv_obj_set_size(btn, width, height);
      Button button;
      button.id = id;
      button.button = btn;
      button.label = txt;
      button.set = 1;
      button.callback = callback;
      if (button_ptr) {
         *button_ptr = button;
      }
      lv_obj_add_event_cb(btn, redirect_cb, LV_EVENT_CLICKED, button_ptr);
      return button;
}

void update_player(DisPlayer* player, PlayerDisplayConfig config) {
   change_player_maxHealth(player, config.healthMax);
   change_player_turn(player, config.turn);
   change_player_stats(player, config.name, config.health);
   // set_user_sprite(player, player->sprite_img);
}


// void lvgl_update_thread(void) {
//    while (1) {
//       if (scene) {
//          lv_timer_handler();
//       }
//       k_sleep(K_MSEC(10));
//    }
// }

// #define RESTART_INTERVAL_MS 200
// #define STACK_SIZE 1024
// #define THREAD_PRIORITY 5
// #define MAX_WAIT_US 30000
// K_THREAD_DEFINE(lvgl_update, STACK_SIZE,
//     lvgl_update_thread, NULL, NULL, NULL,
//     THREAD_PRIORITY, 0, 0);

#endif

BattleSceneConfig* init_battle_scene(BattleSceneConfig* config) {
#ifndef CONFIG_LVGL
   LOG_DBG("screen not available");
#else
   LOG_INF("[%s] %s vs %s [%s]",
      config->me.name,
      config->me.spriteName,
      config->opponent.spriteName,
      config->opponent.name);
   lv_obj_clean(display.scr);

   initialise_player(&user, &config->me);
   initialise_player(&opponent, &config->opponent);

   for (int i = 0; i < config->buttonCount; i++) {
      int x = 0;
      int y = BUTTON_POS_1Y + (BUTTON_HEIGHT * (i - 1));
      if (i == 0) {
         x = 85;
         y = BUTTON_POS_1Y;
      }

      buttons[i] = create_button(config->buttons[i].callback, display.scr,
         x, y,
            config->buttons[i].label,
            80, 45, config->buttons[i].id, buttons + i
         );
   }

   buttonCount = config->buttonCount;

   display.user_hlth = lv_label_create(display.scr);
   display.opponent_hlth = lv_label_create(display.scr);
   lv_obj_set_style_text_color(display.user_hlth, lv_color_black(), 0);
   lv_obj_set_style_text_color(display.opponent_hlth, lv_color_black(), 0);
   display_player_health(&user);
   display_player_health(&opponent);
   display.user_hlthbar = set_healthbar(&user);
   display.opp_hlthbar = set_healthbar(&opponent);

   /* creating the background */
   static lv_style_t style;
   lv_style_init(&style);
   lv_style_set_bg_color(&style, lv_palette_main(LV_PALETTE_YELLOW));
   lv_style_set_bg_opa(&style, LV_OPA_COVER);
   lv_obj_t *img = lv_img_create(display.scr);
   lv_obj_add_style(img, &style, 0);
   lv_img_set_src(img, &battleground_b);
   lv_obj_center(img);
   lv_obj_move_background(img);

   scene = 1;
#endif
   return config;
}

/* button mask will set action 1 based on bit 0, 2 based on bit 1, so on and so forth
   0 = greyed out, 1 = available
*/
void update_battle_scene(BattleSceneConfig* config) {
#ifndef CONFIG_LVGL
   LOG_DBG("screen not available");
#else
   if (scene != 1) {
      init_battle_scene(config);
   }

   for (int i = 0; i < config->buttonCount; i++) {
      change_player_action(config->buttons[i].label, i, buttons + 1);
      set_button_mode(buttons + i, config->buttons[i].on);
   }
   buttonCount = config->buttonCount;

   update_player(&user, config->me);
   update_player(&opponent, config->opponent);
#endif
}

ConnectionSceneConfig* init_connections_scene(ConnectionSceneConfig* config) {
#ifndef CONFIG_LVGL
   LOG_DBG("screen not available");
#else
   lv_obj_clean(display.scr);

   for (int i = 0; i < config->buttonCount && i < 9; i++) {
      int x = 10 + (i / 3) * 100;
      int y = BUTTON_POS_1Y + (i % 3) * BUTTON_HEIGHT;
      buttons[i] = create_button(config->buttons[i].callback,
         display.scr,
         x, y,
         config->buttons[i].label,
         90, 45,
         config->buttons[i].id,
         buttons + i);
      set_button_mode(buttons + i, config->buttons[i].on);
   }
   buttonCount = config->buttonCount;

   scene = 2;
#endif
   return config;
}

void update_connections_scene(ConnectionSceneConfig* config) {
   init_connections_scene(config);
}

//TODO: make a list of buttons for each action DONE
//TODO: set a function which changes the scene NOT NEEDED
//TODO: create a function which sets player health DONE
//TODO: create a function which changes player name DONE
//TODO: create a function which recursively makes buttons r/r + c/c KINDA IMPOSSIBLE
//TODO: ensure that actions are blocked given player.turn is false DONE
//TODO: create a function which loads an image into the scene DONE

/* Adds the background image, icon image and coordinates.
*/
void init_screen(void) {
#ifndef CONFIG_LVGL
   LOG_WRN("screen not available");
#else
   const struct device *display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
   if (!device_is_ready(display_dev)) {
      return;
   }

   display.scr = lv_scr_act();

   display_blanking_off(display_dev);
   lv_timer_handler();
#endif
}


int update_screen(void) {
#ifdef CONFIG_LVGL
      return (int) lv_timer_handler();
#else
   return 10;
#endif
}