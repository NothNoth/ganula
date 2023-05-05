#include "setup.h"
#include "controls.h"
#include "display.h"
#include <math.h>
#include <string.h>


#ifdef _GRANULA_TESTS_
  #include "granula_tests_stubs.h"
#else
  #include "ntm.h"
  #include <DueTimer.h>
#endif
#include "midi.h"
#include "gsynth.h"
#include "menu.h"
#include "granula.h"

gmode_t mode;
int last_pot_value;

#define CUSTOM_REC_SAMPLE_SIZE 128
#define CUSTOM_REC_SAMPLE_POINT_MS 120
#define CUSTOM_REC_SAMPLE_POT_MARGIN 128
int custom_rec_idx;
int custom_rec_ts;
unsigned short customrecsample[CUSTOM_REC_SAMPLE_SIZE];

int adsr_select_idx;

void pot_changed(int value);
void bt1_pressed(int unused);
void bt2_pressed(int unused);


void setup() {
  last_pot_value = 0;

  //Setup debug print on serial port
  debug_setup(115200);
  debug_print("Granula");

  pinMode(AUDIO_PIN, OUTPUT);
  mode = GMODE_RUN;

  //Setup Display
  display_setup();

  //Setup MIDI receive
  midi_setup();
  midi_register_noteon_cb(note_on);
  midi_register_noteoff_cb(note_off);
  
  //Setup controls
  controls_setup();
  controls_register_pot_cb(pot_changed);
  controls_register_bt1_cb(bt1_pressed);
  controls_register_bt2_cb(bt2_pressed);

  //Menu
  menu_setup();

  //Setup synth
  gsynth_setup();
  Timer3.attachInterrupt(dacoutput).setFrequency(SAMPLE_RATE).start();
}


void loop() {
  controls_loop();
  display_loop();
  midi_loop();
  menu_loop();
  gsynth_loop();

  if (mode == GMODE_CUSTOM_REC) {
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
        gmode_switch(GMODE_RUN);
      }
    }
  }
}

gmode_t gmode_get() {
  return mode;
}

void gmode_switch(gmode_t new_mode) {

  //Stop previous mode
  switch (mode) {
    case GMODE_RUN:
      gsynth_enable(false);
    break;
    case GMODE_CUSTOM_POTSYNC:
    break;
    case GMODE_CUSTOM_REC:
    break;
    case GMODE_ADSR:
    break;
    case GMODE_MAX:
    break;
  }

  //Setup new mode
  switch (new_mode) {
    case GMODE_RUN:
      gsynth_enable(true);
    break;
    case GMODE_CUSTOM_POTSYNC:
      display_potsync(last_pot_value);
    break;
    case GMODE_CUSTOM_REC:
      custom_rec_idx = 0;
      custom_rec_ts = millis();
      memset(customrecsample, 0x00, CUSTOM_REC_SAMPLE_SIZE*sizeof(unsigned short));
      display_rec(custom_rec_idx, last_pot_value, customrecsample);
    break;
    case GMODE_ADSR:
      adsr_t adsr;
      gsynth_enable(true);
      adsr_select_idx = 0;
      gsynth_get_adsr(&adsr);
      display_adsr(adsr.a_ms, adsr.d_ms, adsr.s, adsr.r_ms, true, false, false, false);
    break;
    case GMODE_MAX:
      return;
    break;
  }

  mode = new_mode;
}


void pot_changed(int value) {
  menu_pot(value);
  
  last_pot_value = value;
  switch (mode) {
    case GMODE_CUSTOM_POTSYNC:
      if (abs(value - POT_RANGE/2.0) < 5) { //Done !
        gmode_switch(GMODE_CUSTOM_REC);
        return;
      }
      display_potsync(value);
    break;
    case GMODE_CUSTOM_REC:
    break;
    case GMODE_ADSR:
    {
      adsr_t adsr;
      gsynth_get_adsr(&adsr);
      switch (adsr_select_idx) {
        case 0: //Attack
          adsr.a_ms = (int)((float)value *1000.0/(float)POT_RANGE);
        break;
        case 1: //Decay
          adsr.d_ms = (int)((float)value *1000.0/(float)POT_RANGE);
        break;
        case 2: //Sustain
          adsr.s = ((float)value/(float)POT_RANGE);
        break;
        case 3: //Release
          adsr.r_ms = (int)((float)value *1000.0/(float)POT_RANGE);
        break;
      }
      gsynth_set_adsr(adsr.a_ms, adsr.d_ms, adsr.s, adsr.r_ms);
      display_adsr(adsr.a_ms, adsr.d_ms, adsr.s, adsr.r_ms, adsr_select_idx==0?true:false, adsr_select_idx==1?true:false, adsr_select_idx==2?true:false, adsr_select_idx==3?true:false);

    }
    break;
    case GMODE_RUN:
    break;
    case GMODE_MAX:
    break;
  }

}


void bt1_pressed(int unused) {
  if (mode == GMODE_ADSR) {
    gmode_switch(GMODE_RUN);
    return;
  }
  menu_flip();
}

void bt2_pressed(int unused) {
  if (mode == GMODE_ADSR) {
    adsr_t adsr;
    gsynth_get_adsr(&adsr);

    adsr_select_idx = (adsr_select_idx+1)%4;
    display_adsr(adsr.a_ms, adsr.d_ms, adsr.s, adsr.r_ms, adsr_select_idx==0?true:false, adsr_select_idx==1?true:false, adsr_select_idx==2?true:false, adsr_select_idx==3?true:false);
    return;
  }
  menu_select();
}
