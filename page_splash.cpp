#include "page_splash.hpp"
#include "display.h"

#ifdef _GRANULA_TESTS_
  #include "granula_tests_stubs.h"
#else
  #include "ntm.h"
#endif
PageSplash::PageSplash() {

}


void PageSplash::enter(){
  debug_print("Entering Splash");
  display_splash();
}

void PageSplash::leave(){
  display_clear();
}

void PageSplash::loop() {
  
}

void PageSplash::button1_pressed(){
  
}
void PageSplash::button2_pressed(){
  
}

void PageSplash::pot_changed(int value) {

}

