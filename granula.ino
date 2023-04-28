#include "setup.h"
#include "ntm.h"
#include "controls.h"
#include "display.h"
#include <DueTimer.h>
#include "midi.h"
#include "gsynth.h"



typedef enum {
  GMODE_RUN = 0,
  GMODE_CUSTOM_POTSYNC = 1,
  GMODE_CUSTOM_REC = 2,
  GMODE_MAX
} gmode_t;

gmode_t mode;
int last_pot_value;

#define CUSTOM_REC_SAMPLE_SIZE 128
int custom_rec_idx;
int custom_rec_ts;
unsigned char customrecsample[CUSTOM_REC_SAMPLE_SIZE];

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

  //Setup synth
  gsynth_setup();
  Timer3.attachInterrupt(dacoutput).setFrequency(SAMPLE_RATE).start();
}


void loop() {
  controls_loop();
  display_loop();
  midi_loop();

  if (mode == GMODE_CUSTOM_REC) {
    if (millis() - custom_rec_ts > 120) {
      custom_rec_idx++;

      unsigned char potpos;
      if (last_pot_value < 128) {
        potpos = 0;
      } else if (last_pot_value > 384) {
        potpos = 255;
      } else {
        potpos = last_pot_value - 128;
      }
      
      display_rec(custom_rec_idx, potpos, customrecsample);
      customrecsample[custom_rec_idx] = potpos;
      custom_rec_ts = millis();

      //Done
      if (custom_rec_idx == CUSTOM_REC_SAMPLE_SIZE) {
        gsynth_save_custom(customrecsample+10, CUSTOM_REC_SAMPLE_SIZE-20);
        gmode_switch(GMODE_RUN);
      }
    }
  }
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
      memset(customrecsample, 0x00, CUSTOM_REC_SAMPLE_SIZE);
      display_rec(custom_rec_idx, last_pot_value, customrecsample);
    break;
  }

  mode = new_mode;
}


void pot_changed(int value) {
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
      //display_rec(custom_rec_idx, value, customrecsample);
    break;
  }
}


void bt1_pressed(int unused) {
  gsynth_nextwave();
}

void bt2_pressed(int unused) {

  switch (mode) {
    case GMODE_RUN:
      gmode_switch(GMODE_CUSTOM_POTSYNC);
    break;
    case GMODE_CUSTOM_POTSYNC:
      gmode_switch(GMODE_RUN);
    break;
    case GMODE_CUSTOM_REC:
      gmode_switch(GMODE_RUN);
    break;
  }
}
