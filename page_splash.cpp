#include "page_splash.hpp"
#include "display.h"
#include "ntm.h"

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

