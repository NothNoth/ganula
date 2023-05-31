#include "page_home.hpp"
#include "display.h"
#include "granula.h"
#include "ntm.h"

PageHome::PageHome() {

}


void PageHome::enter(){
  display_text("MENU  |    ", 2, false);
  debug_print("Entering Home");
}

void PageHome::leave(){
  display_clear();
}

void PageHome::loop() {
  
}

void PageHome::button1_pressed(){
  gmode_switch(PAGE_MENU);
}

void PageHome::button2_pressed(){
  
}

void PageHome::pot_changed(int value) {

}

