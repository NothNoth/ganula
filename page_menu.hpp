#ifndef _PAGE_MENU_H_
#define _PAGE_MENU_H_
#include "pages.hpp"



#define MAX_MENU_ITEMS 6
typedef enum {
  menu_tag_waveform = 0,
  menu_tag_waveform_new = 1,
  menu_tag_enveloppe = 2,
  menu_tag_reset = 9,

  menu_tag_waveform_sin = 3,
  menu_tag_waveform_square = 4,
  menu_tag_waveform_triangle = 5,
  menu_tag_waveform_saw = 6,
  menu_tag_waveform_custom1 = 7,
  menu_tag_waveform_isaw = 8,
} menu_tag_t;

typedef struct 
{
  char name[16];
  menu_tag_t tag;
} menu_item_t;

typedef struct {
  menu_item_t items[MAX_MENU_ITEMS];
  int menu_items_count;
  int menu_idx;
} menu_t;

class PageMenu : public virtual Page{
  public:
    PageMenu();
    void enter();
    void leave();
    void loop();
    void button1_pressed();
    void button2_pressed();
    void pot_changed(int value);
  private:
    menu_t root;
    menu_t waveforms;
    menu_t *current;

    void refresh();



};

#endif