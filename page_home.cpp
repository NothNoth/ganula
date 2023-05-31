#include "page_home.hpp"
#include "display.h"
#include "granula.h"

#ifdef _GRANULA_TESTS_
  #include "granula_tests_stubs.h"
#else
  #include "ntm.h"
#endif
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

