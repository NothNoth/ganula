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


void setup() {

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
}



void pot_changed(int value) {
  debug_print(value);
  switch (mode) {
    case GMODE_CUSTOM_POTSYNC:
      if (abs(value - POT_RANGE/2.0) < 5) { //Done !
        mode = GMODE_CUSTOM_REC;
        display_rec();
        return;
      }
      display_potsync(value);
    break;
  }
}


void bt1_pressed(int unused) {
  gsynth_nextwave();
}

void bt2_pressed(int unused) {

  switch (mode) {
    case GMODE_RUN:
      gsynth_enable(false);
      mode =  GMODE_CUSTOM_POTSYNC;
      display_potsync(0);
    break;
    case GMODE_CUSTOM_POTSYNC:
      gsynth_enable(true);
      mode =  GMODE_RUN;
    break;
    case GMODE_CUSTOM_REC:
      gsynth_enable(true);
      mode =  GMODE_RUN;
    break;
  }
}
