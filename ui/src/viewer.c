#include "viewer.h"

LOG_MODULE_DECLARE(app);


/* declaring all possible images/characters available*/

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
LV_IMG_DECLARE(cleffback);
LV_IMG_DECLARE(mewfront);
LV_IMG_DECLARE(mewback);

/* end of declarations */

Display display;
Player user;
Player opponent;

Button b1;
Button b2;
Button b3;
Button b4;
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

/* SIORYN - ALL CALLBACKS HERE */
static void action_1(lv_event_t *e)
{

   Button *btn = (Button *)lv_event_get_user_data(e);
   if (btn->set) {
      change_player_health(&user, 50);
      change_player_health(&opponent, 75);
      LOG_INF("action 1");
   }

}

static void action_2(lv_event_t *e)
{
   Button *btn = (Button *)lv_event_get_user_data(e);
   if (btn->set) {
      change_player_action(&user, "Tickle", 0, &b1);
      LOG_INF("action 2");
   }

}

static void action_3(lv_event_t *e)
{
   Button *btn = (Button *)lv_event_get_user_data(e);
   if (btn->set) {
      Player new;
      initialise_player(&new, USER, "asshat", 100, generic_actions_2);
      change_battle_scene(&new, &opponent);
      LOG_INF("action 3");
   }

}

static void action_4(lv_event_t *e)
{
   Button *btn = (Button *)lv_event_get_user_data(e);
   if (btn->set) {
      set_button_mode(&b1, 0);
      set_user_sprite(&user, &charback);
      LOG_INF("action 4");
   }

}

/* NO MORE CALLBACKS */

void change_healthbar(Player *player) {
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

void display_player_health(Player *player) {
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

void initialise_player(Player *player, int playerNum, const char *name, int maxHealth, char actions[4][20]) {
   player->playerNum = playerNum;
   snprintf(player->pName, sizeof(player->pName), "%s", name);
   player->maxHealth = maxHealth;
   player->health = maxHealth;
   for (int i = 0; i < 4; i++) {
      snprintf(player->actions[i], sizeof(player->actions[i]), "%s", actions[i]);
   }
   player->sprite = NULL;
   player->turn = 1;
   player->sprite_img = NULL;
}

void change_player_name(Player *player, const char *name) {
   snprintf(player->pName, sizeof(player->pName), "%s", name);
   change_player_health(player, player->health);
}

void change_player_health(Player *player, int health) {
   player->health = health;
   if(player->playerNum == USER) {
      change_healthbar(player);
   } else if (player->playerNum == OPPONENT) {
      change_healthbar(player);
   }
   display_player_health(player);
}

void change_player_stats(Player *player, const char *name, int health) {
   snprintf(player->pName, sizeof(player->pName), "%s", name); 
   player->health = health;
   if(player->playerNum == USER) {
      change_healthbar(player);
   } else if (player->playerNum == OPPONENT) {
      change_healthbar(player);
   }
   display_player_health(player);
}

void change_player_turn(Player *player, int turn) {
   player->turn = turn;
}

void change_player_maxHealth(Player *player, int maxHealth) {
   player->maxHealth = maxHealth;
}

void change_button_label(Button *button, char *label) {
   lv_label_set_text(button->label, label);
}

void set_button_mode(Button *button, int mode) {
   button->set = mode;
   if (!mode) {
      lv_obj_set_style_bg_color(button->button, lv_palette_main(LV_PALETTE_GREY), 0);
   } else {
      lv_obj_set_style_bg_color(button->button, lv_palette_main(LV_PALETTE_BLUE), 0); 
   }
}

void change_player_action(Player *player, char *action, int index, Button *button) {
   if (index < 4) {
      snprintf(player->actions[index], sizeof(player->actions[index]), "%s", action);
      change_button_label(button, action);
   } else {
      LOG_ERR("Index out of bounds");
   }

}

lv_obj_t* set_healthbar(Player *player)
{
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

Button create_button(void (*callback)(lv_event_t *), lv_obj_t* screen, int x, int y, char* label, int width, int height, Button *button_ptr) {
      lv_obj_t *btn = lv_button_create(screen);
      lv_obj_t *txt = lv_label_create(btn);
      lv_obj_align(btn, LV_ALIGN_DEFAULT, x, y);
      lv_obj_add_event_cb(btn, callback, LV_EVENT_CLICKED, button_ptr);
      lv_label_set_text(txt, label);
      lv_obj_align(txt, LV_ALIGN_CENTER, 0, 0);
      lv_obj_set_size(btn, width, height);
      Button button;
      button.button = btn;
      button.label = txt;
      button.set = 1;
      if (button_ptr) {
         *button_ptr = button;
      }
      return button;
}

void set_user_sprite(Player *player, const lv_img_dsc_t *img) {
   if (player->sprite) {
      lv_obj_del(player->sprite);
      player->sprite = NULL;
   }
   player->sprite_img = img;      
   player->sprite = lv_img_create(display.scr);
   lv_img_set_src(player->sprite, img);
   if (player->playerNum == USER) {
      lv_obj_align(player->sprite, LV_ALIGN_DEFAULT, P1X, P1Y - 76);
   } else if (player->playerNum == OPPONENT) {
      lv_obj_align(player->sprite, LV_ALIGN_DEFAULT, P2X, P2Y);
   }
}

void set_battle_scene(void) {
   scene = 1;
   lv_obj_clean(display.scr);
   b1 = create_button(action_1, display.scr, 0, BUTTON_POS_1Y, user.actions[0], 80, 30, &b1);
   b2 = create_button(action_2, display.scr, 0, BUTTON_POS_1Y + BUTTON_HEIGHT, user.actions[1], 80, 30, &b2);
   b3 = create_button(action_3, display.scr, 0, BUTTON_POS_1Y + (BUTTON_HEIGHT * 2), user.actions[2], 80, 30, &b3);
   b4 = create_button(action_4, display.scr, 0, BUTTON_POS_1Y + (BUTTON_HEIGHT * 3), user.actions[3], 80, 30, &b3);
   display.user_hlth = lv_label_create(display.scr);
   display.opponent_hlth = lv_label_create(display.scr);
   lv_obj_set_style_text_color(display.user_hlth, lv_color_black(), 0);
   lv_obj_set_style_text_color(display.opponent_hlth, lv_color_black(), 0);
   display_player_health(&user);
   display_player_health(&opponent);
   display.user_hlthbar = set_healthbar(&user);
   display.opp_hlthbar = set_healthbar(&opponent);
   static lv_style_t style;
   lv_style_init(&style);
   lv_style_set_bg_color(&style, lv_palette_main(LV_PALETTE_YELLOW));
   lv_style_set_bg_opa(&style, LV_OPA_COVER);
   lv_obj_t *img = lv_img_create(display.scr);
   lv_obj_add_style(img, &style, 0);
   lv_img_set_src(img, &battleground_b);
   lv_obj_center(img);
   lv_obj_move_background(img);
   set_user_sprite(&user, &bulbaback);
   set_user_sprite(&opponent, &bulbafront);
}

void change_battle_scene(Player *P1, Player *P2) {
   if (scene != 1) { // battle scene was already set, no need to create a new one
      set_battle_scene();
   } 
   change_player_action(&user, P1->actions[0], 0, &b1); 
   change_player_action(&user, P1->actions[1], 1, &b2);    
   change_player_action(&user, P1->actions[2], 2, &b3); 
   change_player_action(&user, P1->actions[3], 3, &b4);
   change_player_maxHealth(&user, P1->maxHealth);
   change_player_maxHealth(&opponent, P2->maxHealth);
   change_player_turn(&user, P1->turn);
   change_player_turn(&opponent, P2->turn);
   change_player_stats(&user, P1->pName, P1->health);
   change_player_stats(&opponent, P2->pName, P2->health);
   set_user_sprite(&user, P1->sprite_img);
   set_user_sprite(&opponent, P2->sprite_img);
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
void create_screen(void) {
   initialise_player(&user, USER, "player1", 100, generic_actions);
   initialise_player(&opponent, OPPONENT, "player2", 100, generic_actions);
   user.turn = 1;
   opponent.turn = 0;
   display.scr = lv_scr_act();
   set_battle_scene();
}