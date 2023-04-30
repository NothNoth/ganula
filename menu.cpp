#include "menu.h"
#include "display.h"
#include "ntm.h"
#include "gsynth.h"
#include "granula.h"
#include <string.h>

bool menu_shown;

#define MAX_MENU_ITEMS 4
typedef enum {
  menu_tag_waveform = 0,
  menu_tag_waveform_new = 1,
  menu_tag_enveloppe = 2,

  menu_tag_waveform_sin = 3,
  menu_tag_waveform_square = 4,
  menu_tag_waveform_triangle = 5,
  menu_tag_waveform_saw = 6,
  menu_tag_waveform_custom1 = 7,

} menu_tag_t;

typedef struct 
{
  char name[16];
  menu_tag_t tag;
} menu_item_t;

typedef struct {
  menu_item_t items[MAX_MENU_ITEMS];
  int menu_idx;
} menu_t;

menu_t root;
menu_t waveforms;

menu_t *current;


void menu_refresh();

void menu_setup() {
  menu_shown = false;

//Root menu
  root.menu_idx = 0;
  memset(root.items, 0x00, MAX_MENU_ITEMS * sizeof(menu_item_t));
  strcpy(root.items[0].name, "Waveform    ");
  root.items[0].tag = menu_tag_waveform;
  strcpy(root.items[1].name, "New waveform");
  root.items[1].tag = menu_tag_waveform_new;
  strcpy(root.items[2].name, "Enveloppe   ");
  root.items[2].tag = menu_tag_enveloppe;
  strcpy(root.items[3].name, "blop        ");
  root.items[3].tag = menu_tag_enveloppe;
  
//Waveforms menu
  waveforms.menu_idx = 0;
  memset(waveforms.items, 0x00, MAX_MENU_ITEMS * sizeof(menu_item_t));
  strcpy(waveforms.items[0].name, "Sin     ");
  waveforms.items[0].tag = menu_tag_waveform_sin;
  strcpy(waveforms.items[1].name, "Square  ");
  waveforms.items[1].tag = menu_tag_waveform_square;
  strcpy(waveforms.items[2].name, "Triangle");
  waveforms.items[2].tag = menu_tag_waveform_triangle;
  strcpy(waveforms.items[3].name, "Custom 1");
  waveforms.items[3].tag = menu_tag_waveform_custom1;

  current = &root;
}

void menu_loop() {

}

void menu_flip() {
  menu_shown = !menu_shown;
  current = &root;

  menu_refresh();
}

void menu_refresh() {
  int next_idx, next_next_idx;
  if (menu_shown == false) {
    return;
  }

  display_clear();
  next_idx = current->menu_idx + 1;
  next_next_idx = next_idx + 1;

  display_text(current->items[current->menu_idx].name, 0, true);
  if (next_idx < MAX_MENU_ITEMS) {
    display_text(current->items[next_idx].name, 1, false);
  }
  if (next_next_idx < MAX_MENU_ITEMS) {
    display_text(current->items[next_next_idx].name, 2, false);
  }
  
}

void menu_select() {
  debug_print(current->items[current->menu_idx].name);
  switch (current->items[current->menu_idx].tag) {
    case menu_tag_waveform:
      current = &waveforms;
      waveforms.menu_idx = 0;
      menu_refresh();
    break;
    case menu_tag_waveform_new:
      gmode_switch(GMODE_CUSTOM_POTSYNC);
      menu_flip();
      return;
    break;
    case menu_tag_enveloppe:
    break;
    case menu_tag_waveform_sin:
      gsynth_select_wave(WAVE_SIN);
      menu_flip();
    break;
    case menu_tag_waveform_triangle:
      gsynth_select_wave(WAVE_TRIANGLE);
      menu_flip();
    break;
    case menu_tag_waveform_square:
      gsynth_select_wave(WAVE_SQUARE);
      menu_flip();
    break;
    case menu_tag_waveform_saw:
      gsynth_select_wave(WAVE_SAW);
      menu_flip();
    break;
    case menu_tag_waveform_custom1:
      gsynth_select_wave(WAVE_CUSTOM);
      menu_flip();
    break;
  }
}


void menu_pot(int value) {
  if (menu_shown == false) {
    return;
  }
  // (512 -> MAX_ITEMS)
  int new_idx = value * (float)(MAX_MENU_ITEMS) / 512.0;
  if (new_idx == current->menu_idx) {
    return;
  }
  current->menu_idx = new_idx;
  menu_refresh();
}
