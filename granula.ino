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
  debug_setup(115200);
  debug_print("Granula");

  midi_setup();
  midi_register_noteon_cb(note_on);
  midi_register_noteoff_cb(note_off);
  
  display_setup();
  controls_setup();
  controls_register_pot_cb(pot_changed);
  controls_register_bt1_cb(bt1_pressed);
  controls_register_bt2_cb(bt2_pressed);

  pinMode(AUDIO_PIN, OUTPUT);

  Timer3.attachInterrupt(dacoutput).setFrequency(SAMPLE_RATE).start();

  mode = GMODE_RUN;
}

void loop() {
  controls_loop();
  display_loop();
  midi_loop();
}



void pot_changed(int value) {

}


void bt1_pressed(int unused) {
  gsynth_nextwave();
}

void bt2_pressed(int unused) {
  gsynth_enable(false);
}
