#include "menu.h"
#include "display.h"
#include "ntm.h"
#include "gsynth.h"
#include "granula.h"
#include <string.h>

bool menu_shown;
int menu_idx;

#define MAX_MENU_ITEMS 4
typedef enum {
  menu_tag_waveform = 0,
  menu_tag_waveform_new = 1,
  menu_tag_enveloppe = 2
} menu_tag_t;

typedef struct 
{
  char name[16];
  menu_tag_t tag;
} menu_item_t;

menu_item_t items[MAX_MENU_ITEMS];

void menu_refresh();


void menu_setup() {
  menu_shown = false;
  memset(items, 0x00, MAX_MENU_ITEMS * sizeof(menu_item_t));
  strcpy(items[0].name, "Waveform    ");
  items[0].tag = menu_tag_waveform;
  strcpy(items[1].name, "New waveform");
  items[0].tag = menu_tag_waveform_new;
  strcpy(items[2].name, "Enveloppe   ");
  items[0].tag = menu_tag_enveloppe;
  strcpy(items[3].name, "blop        ");
  items[0].tag = menu_tag_enveloppe;
  
  menu_idx = 0;
}

void menu_loop() {

}

void menu_flip() {
  menu_shown = !menu_shown;
  menu_refresh();
}

void menu_refresh() {
  int next_idx, next_next_idx;
  if (menu_shown == false) {
    return;
  }

  display_clear();
  next_idx = menu_idx + 1;
  next_next_idx = next_idx + 1;

  display_text(items[menu_idx].name, 0, true);
  if (next_idx < MAX_MENU_ITEMS) {
    display_text(items[next_idx].name, 1, false);
  }
  if (next_next_idx < MAX_MENU_ITEMS) {
    display_text(items[next_next_idx].name, 2, false);
  }
  
}

void menu_select() {
  debug_print(items[menu_idx].name);
  switch (items[menu_idx].tag) {
    case menu_tag_waveform:
      gsynth_nextwave();
      gmode_switch(GMODE_RUN);
      menu_flip();
      return;
    break;
    case menu_tag_waveform_new:
      gmode_switch(GMODE_CUSTOM_POTSYNC);
      menu_flip();
      return;
    break;
    case menu_tag_enveloppe:
    break;
  }
}


void menu_pot(int value) {
  if (menu_shown == false) {
    return;
  }
  // (512 -> MAX_ITEMS)
  int new_idx = value * (float)(MAX_MENU_ITEMS) / 512.0;
  if (new_idx == menu_idx) {
    return;
  }
  menu_idx = new_idx;
  menu_refresh();
}
