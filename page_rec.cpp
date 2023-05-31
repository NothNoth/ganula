#include "page_rec.hpp"
#include "display.h"
#include "granula.h"
#include "setup.h"
#include "gsynth.h"
#include <string.h>
#include <math.h>

#ifdef _GRANULA_TESTS_
  #include "granula_tests_stubs.h"
#else
  #include "ntm.h"
  #include "Arduino.h"
#endif


PageRec::PageRec() {

}

void PageRec::enter(){
  debug_print("Entering REC");
  last_pot_value = 0;
  pot_sync = true;
  display_potsync(last_pot_value);

}

void PageRec::leave(){
  display_clear();
}

void PageRec::loop() {
  if (pot_sync) {
    return;
  }

  if (millis() - custom_rec_ts > CUSTOM_REC_SAMPLE_POINT_MS) {
      custom_rec_idx++;

      int fact = MAX_DAC / POT_RANGE;
      int pot_value = (POT_RANGE - last_pot_value)*fact;
      display_rec(custom_rec_idx, pot_value, customrecsample);
      customrecsample[custom_rec_idx] = pot_value;
      custom_rec_ts = millis();

      //Done
      if (custom_rec_idx == CUSTOM_REC_SAMPLE_SIZE) {
        gsynth_save_custom(customrecsample+10, CUSTOM_REC_SAMPLE_SIZE-20);
        gmode_switch(PAGE_HOME);
      }
    }
}

void PageRec::button1_pressed(){
  
}
void PageRec::button2_pressed(){
  
}

void PageRec::pot_changed(int value) {
  last_pot_value = value;

  if (!pot_sync) {
    return;
  }

  if (abs(value - POT_RANGE/2.0) < 5) { //Done !
      pot_sync = false;
      custom_rec_idx = 0;
      custom_rec_ts = millis();
      memset(customrecsample, 0x00, CUSTOM_REC_SAMPLE_SIZE*sizeof(unsigned short));
      display_rec(custom_rec_idx, last_pot_value, customrecsample);
      return;
  }
  display_potsync(value);
}

