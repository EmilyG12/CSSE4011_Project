#include "viewer.h"
#define POKEMON_COUNT 8
LOG_MODULE_DECLARE(app);

#if defined(CONFIG_LVGL)
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
LV_IMG_DECLARE(clefback);
LV_IMG_DECLARE(mewfront);
LV_IMG_DECLARE(mewback);

Sprite charmander = {"charmander", &charfront, &charback};
Sprite squirtle = {"squirtle", &squirtfront, &squirtback};
Sprite bulbasaur = {"bulbasaur", &bulbafront, &bulbaback};
Sprite pidgey = {"pidgey", &pidgefront, &pidgeback};
Sprite pikachu = {"pikachu", &pikafront, &pikaback};
Sprite clefairy = {"clefairy", &cleffront, &clefback};
Sprite mew = {"mew", &mewfront, &mewback};
Sprite poliwhirl = {"poliwhirl", &polifront, &poliback};
Sprite *sprites[POKEMON_COUNT] = {&charmander, &squirtle, &bulbasaur, &pidgey, &pikachu, &clefairy, &mew, &poliwhirl};

/* end of declarations */
#endif

Display display;
DisPlayer user;
DisPlayer opponent;

Button b1;
Button b2;
Button b3;
Button b4;

/* CONNECTION BUTTONS -- button.setting could be used to imply which connection number is being referred to -- putting that ID into callback*/
Button c1;
Button c2;
Button c3;
Button c4;
Button c5;
Button c6;
Button c7;
Button c8;
Button c9;

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

#if defined(CONFIG_LVGL)
/* SIORYN - ALL CALLBACKS HERE */
static void action_1(int e)
{

   // Button *btn = (Button *)lv_event_get_user_data(e);
   // if (btn->set) {
   change_player_health(&user, 50);
   change_player_health(&opponent, 75);
   LOG_INF("action 1");
   // }

}

static void action_2(int e)
{

   change_connection_scene(0b101000101);

}

static void action_3(int e)
{
   DisPlayer newPlayer;
   initialise_player(&newPlayer, USER, "charmander", 100, generic_actions_2);
   change_battle_scene(&newPlayer, &opponent, 0b1101);
   LOG_INF("action 3");

}

static void action_4(int e)
{

   set_button_mode(&b1, 0);
   set_user_sprite(&user, &charback);
   LOG_INF("action 4");

}

static void connection(int e) {
   //knowing e is an int referring to the connection number, something something something connect to connection 'x' through some math to get what 'x' is knowing the ID
   //assume the connection list is in order of connection number

   change_battle_scene(&user, &opponent, 0b1111);
}

/* NO MORE CALLBACKS */

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

void set_player_sprite(DisPlayer *player) {
   int found = 0;
   for (int i = 0; i < POKEMON_COUNT; i++) {
      if (!strcmp(player->pName, sprites[i]->name)) {
         if (player->playerNum == OPPONENT) {
            player->sprite_img = sprites[i]->front;
         } else if (player->playerNum == USER) {
            player->sprite_img = sprites[i]->back;
         }
         found = 1;
         break;
      }
   }
   if (found) {
      set_user_sprite(player, player->sprite_img);
   } else {
      set_user_sprite(player, &bulbaback);
      LOG_ERR("Sprite not found");
   }
   set_user_sprite(player, player->sprite_img);
}

void initialise_player(DisPlayer *player, int playerNum, const char name[], int maxHealth, char actions[4][20]) {
   player->playerNum = playerNum;
   strncpy(player->pName, name, sizeof(player->pName) - 1);
   player->maxHealth = maxHealth;
   player->health = maxHealth;
   for (int i = 0; i < 4; i++) {
      snprintf(player->actions[i], sizeof(player->actions[i]), "%s", actions[i]);
   }
   player->sprite = NULL;
   player->turn = 1;
   set_player_sprite(player);
}

void change_player_name(DisPlayer *player, const char *name) {
   snprintf(player->pName, sizeof(player->pName), "%s", name);
   change_player_health(player, player->health);
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

void change_player_action(DisPlayer *player, char *action, int index, Button *button) {
   if (index < 4) {
      snprintf(player->actions[index], sizeof(player->actions[index]), "%s", action);
      change_button_label(button, action);
   } else {
      LOG_ERR("Index out of bounds");
   }

}

lv_obj_t* set_healthbar(DisPlayer *player)
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

void redirect_cb(lv_event_t *e) {
   Button *btn = (Button *)lv_event_get_user_data(e);
   if (btn->set) {
      btn->callback(btn->id);
   }
}

Button create_button(void (*callback)(int), lv_obj_t* screen, int x, int y, char* label, int width, int height, int id, Button *button_ptr) {
      lv_obj_t *btn = lv_button_create(screen);
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

void set_user_sprite(DisPlayer *player, const lv_img_dsc_t *img) {
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

   initialise_player(&user, USER, user.pName, user.maxHealth, user.actions);
   initialise_player(&opponent, OPPONENT, opponent.pName, opponent.maxHealth, opponent.actions);

   /* Clear screen, set flags to imply on battle scene*/
   scene = 1;
   lv_obj_clean(display.scr);

   /* Creating buttons for battle scene */
   b1 = create_button(action_1, display.scr, 0, BUTTON_POS_1Y, user.actions[0], 80, 45, ACT1_ID, &b1);
   b2 = create_button(action_2, display.scr, 0, BUTTON_POS_1Y + BUTTON_HEIGHT, user.actions[1], 80, 45, ACT2_ID, &b2);
   b3 = create_button(action_3, display.scr, 0, BUTTON_POS_1Y + (BUTTON_HEIGHT * 2), user.actions[2], 80, 45, ACT3_ID, &b3);
   b4 = create_button(action_4, display.scr, 0, BUTTON_POS_1Y + (BUTTON_HEIGHT * 3), user.actions[3], 80, 45, ACT4_ID, &b4);

   /* setting player and opponent healthbars/health/name text */
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

   /* setting initial sprites */
   set_user_sprite(&user, user.sprite_img);
   set_user_sprite(&opponent, opponent.sprite_img);
}

/* button mask will set action 1 based on bit 0, 2 based on bit 1, so on and so forth
   0 = greyed out, 1 = available
*/
void change_battle_scene(DisPlayer *P1, DisPlayer *P2, uint8_t button_mask) {
   if (scene != 1) { // battle scene was already set, no need to create a new one
      set_battle_scene();
   } 
   change_player_action(&user, P1->actions[0], 0, &b1); 
   change_player_action(&user, P1->actions[1], 1, &b2);    
   change_player_action(&user, P1->actions[2], 2, &b3); 
   change_player_action(&user, P1->actions[3], 3, &b4);
   set_button_mode(&b1, (button_mask >> 0) & 1);
   set_button_mode(&b2, (button_mask >> 1) & 1);
   set_button_mode(&b3, (button_mask >> 2) & 1);
   set_button_mode(&b4, (button_mask >> 3) & 1);
   change_player_maxHealth(&user, P1->maxHealth);
   change_player_maxHealth(&opponent, P2->maxHealth);
   change_player_turn(&user, P1->turn);
   change_player_turn(&opponent, P2->turn);
   change_player_stats(&user, P1->pName, P1->health);
   change_player_stats(&opponent, P2->pName, P2->health);
   set_user_sprite(&user, P1->sprite_img);
   set_user_sprite(&opponent, P2->sprite_img);
}

/* Connections variable basically implies which connections are to be not greyed out: so 010010101 will show that connections 2, 5, 7 
   and 9 are available. 
*/
void change_connection_scene(uint32_t connections) {
   if (scene != 2) {
      LOG_INF("Changing to connection scene");
      lv_obj_clean(display.scr);
      c1 = create_button(connection, display.scr, 10, BUTTON_POS_1Y, "Connect", 90, 45, CONN1_ID , &c1);
      c4 = create_button(connection, display.scr, 10, BUTTON_POS_1Y + BUTTON_HEIGHT, "Connect", 90, 45, CONN2_ID , &c4);
      c7 = create_button(connection, display.scr, 10, BUTTON_POS_1Y + (BUTTON_HEIGHT * 2), "Connect", 90, 45, CONN3_ID , &c7);
      c2 = create_button(connection, display.scr, 110, BUTTON_POS_1Y, "Connect", 90, 45, CONN4_ID , &c2);
      c5 = create_button(connection, display.scr, 110, BUTTON_POS_1Y + BUTTON_HEIGHT, "Connect", 90, 45, CONN4_ID , &c5);
      c8 = create_button(connection, display.scr, 110, BUTTON_POS_1Y + (BUTTON_HEIGHT * 2), "Connect", 90, 45, CONN4_ID , &c8);
      c3 = create_button(connection, display.scr, 210, BUTTON_POS_1Y, "Connect", 90, 45, CONN4_ID , &c3);
      c6 = create_button(connection, display.scr, 210, BUTTON_POS_1Y + BUTTON_HEIGHT, "Connect", 90, 45, CONN4_ID , &c6);
      c9 = create_button(connection, display.scr, 210, BUTTON_POS_1Y + (BUTTON_HEIGHT * 2), "Connect", 90, 45, CONN4_ID , &c9);
      scene = 2;
   }
   for (int i = 0; i < 9; i++) {
      int x = (connections >> i) & 1;
      if (x) {
         switch (i) {
            case 0:
               set_button_mode(&c1, 1);
               change_button_label(&c1, "Available");
               break;
            case 1:
               set_button_mode(&c2, 1);
               change_button_label(&c2, "Available");
               break;
            case 2:
               set_button_mode(&c3, 1);
               change_button_label(&c3, "Available");
               break;
            case 3:
               set_button_mode(&c4, 1);
               change_button_label(&c4, "Available");
               break;
            case 4:
               set_button_mode(&c5, 1);
               change_button_label(&c5, "Available");
               break;
            case 5:
               set_button_mode(&c6, 1);
               change_button_label(&c6, "Available");
               break;
            case 6:
               set_button_mode(&c7, 1);
               change_button_label(&c7, "Available");
               break;
            case 7:
               set_button_mode(&c8, 1);
               change_button_label(&c8, "Available");
               break;
            case 8:
               set_button_mode(&c9, 1);
               change_button_label(&c9, "Available");
               break;
         }
      } else {
         switch (i) {
            case 0:
               set_button_mode(&c1, 0);
               change_button_label(&c1, "Unavailable");
               break;
            case 1:
               set_button_mode(&c2, 0);
               change_button_label(&c2, "Unavailable");
               break;
            case 2:
               set_button_mode(&c3, 0);
               change_button_label(&c3, "Unavailable");
               break;
            case 3:
               set_button_mode(&c4, 0);
               change_button_label(&c4, "Unavailable");
               break;
            case 4:
               set_button_mode(&c5, 0);
               change_button_label(&c5, "Unavailable");
               break;
            case 5:
               set_button_mode(&c6, 0);
               change_button_label(&c6, "Unavailable");
               break;
            case 6:
               set_button_mode(&c7, 0);
               change_button_label(&c7, "Unavailable");
               break;
            case 7:
               set_button_mode(&c8, 0);
               change_button_label(&c8, "Unavailable");
               break;
            case 8:
               set_button_mode(&c9, 0);
               change_button_label(&c9, "Unavailable");
         }
      }
   }
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
   change_connection_scene(0b111000111);

   const struct device *display_dev;

   display_dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_display));
   if (!device_is_ready(display_dev)) {
      return;
   }

   lv_timer_handler();
   display_blanking_off(display_dev);

   while (1) {
      // uint32_t sleep_ms = lV_timer_handler();
      lv_timer_handler();
      k_sleep(K_MSEC(10));
   }
}

#endif