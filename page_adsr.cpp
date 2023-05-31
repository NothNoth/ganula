#include "page_adsr.hpp"
#include "display.h"
#include "setup.h"
#include "granula.h"
#include "gsynth.h"
#ifdef _GRANULA_TESTS_
  #include "granula_tests_stubs.h"
#else
  #include "ntm.h"
#endif

PageADSR::PageADSR() {

}


void PageADSR::enter(){
  adsr_t adsr;
  debug_print("Entering ADSR");
  adsr_select_idx = 0;
  gsynth_get_adsr(&adsr);
  display_adsr(adsr.a_ms, adsr.d_ms, adsr.s, adsr.r_ms, true, false, false, false);
}

void PageADSR::leave(){
  display_clear();
}

void PageADSR::loop() {
  
}

void PageADSR::button1_pressed(){
  gmode_switch(PAGE_HOME);
}

void PageADSR::button2_pressed(){
  adsr_t adsr;
  gsynth_get_adsr(&adsr);
  adsr_select_idx = (adsr_select_idx+1)%4;
  display_adsr(adsr.a_ms, adsr.d_ms, adsr.s, adsr.r_ms, adsr_select_idx==0?true:false, adsr_select_idx==1?true:false, adsr_select_idx==2?true:false, adsr_select_idx==3?true:false);
}

void PageADSR::pot_changed(int value) {
  adsr_t adsr;
  gsynth_get_adsr(&adsr);
  switch (adsr_select_idx) {
    case 0: //Attack
      adsr.a_ms = (int)((float)value *ADSR_MAX_ATTACK_MS/(float)POT_RANGE);
    break;
    case 1: //Decay
      adsr.d_ms = (int)((float)value *ADSR_MAX_DECAY_MS/(float)POT_RANGE);
    break;
    case 2: //Sustain
      adsr.s = (int)((float)value *ADSR_RANGE/(float)POT_RANGE);
    break;
    case 3: //Release
      adsr.r_ms = (int)((float)value *ADSR_MAX_RELEASE_MS/(float)POT_RANGE);
    break;
  }
  gsynth_set_adsr(adsr.a_ms, adsr.d_ms, adsr.s, adsr.r_ms);
  display_adsr(adsr.a_ms, adsr.d_ms, adsr.s, adsr.r_ms, adsr_select_idx==0?true:false, adsr_select_idx==1?true:false, adsr_select_idx==2?true:false, adsr_select_idx==3?true:false);

}

