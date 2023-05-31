#include "setup.h"
#include "controls.h"
#include "display.h"
#include <math.h>
#include <string.h>

#include "pages.hpp"
#include "page_splash.hpp"
#include "page_home.hpp"
#include "page_rec.hpp"
#include "page_menu.hpp"
#include "page_adsr.hpp"

#ifdef _GRANULA_TESTS_
  #include "granula_tests_stubs.h"
#else
  #include "ntm.h"
  #include <DueTimer.h>
#endif
#include "midi.h"
#include "gsynth.h"
#include "granula.h"

gmode_t mode;

//Reset helpers
#define SYSRESETREQ    (1<<2)
#define VECTKEY        (0x05fa0000UL)
#define VECTKEY_MASK   (0x0000ffffUL)
#define AIRCR          (*(uint32_t*)0xe000ed0cUL) // fixed arch-defined address
#define REQUEST_EXTERNAL_RESET (AIRCR=(AIRCR&VECTKEY_MASK)|VECTKEY|SYSRESETREQ)


void pot_changed(int value);
void bt1_pressed(int unused);
void bt2_pressed(int unused);

Page *current_page = NULL;

void setup() {
  //Setup debug print on serial port
  debug_setup(115200);
  debug_print("Granula");

  //Setup synth
  pinMode(AUDIO_PIN, OUTPUT);
  gsynth_setup();

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


  Timer3.attachInterrupt(dacoutput).setFrequency(SAMPLE_RATE).start();

  current_page = NULL;
  gsynth_enable(true);
  gmode_switch(PAGE_SPLASH);
  gmode_switch(PAGE_HOME);
}


void loop() {
  controls_loop();
  display_loop();
  midi_loop();
  gsynth_loop();

  if (current_page != NULL) {
    current_page->loop();
  }
}

gmode_t gmode_get() {
  return mode;
}

void gmode_switch(gmode_t new_mode) {

  if (current_page != NULL) {
    current_page->leave();
    delete(current_page);
  }
  mode = new_mode;

  switch (mode) {
    case PAGE_SPLASH:
      current_page = new PageSplash();
      break;
    case PAGE_HOME:
      current_page = new PageHome();
      break;
    case PAGE_REC:
      current_page = new PageRec();
      break;
    case PAGE_MENU:
      current_page = new PageMenu();
      break;
    case PAGE_ADSR:
      current_page = new PageADSR();
      break;
    default:
      current_page = new PageHome();
      break;
  }
  current_page->enter();
}


void pot_changed(int value) {
  if (current_page != NULL) {
    current_page->pot_changed(value);
  }
}


void bt1_pressed(int unused) {
  if (current_page != NULL) {
    current_page->button1_pressed();
  }
}

void bt2_pressed(int unused) {
  if (current_page != NULL) {
    current_page->button2_pressed();
  }
}


void reset() {
  REQUEST_EXTERNAL_RESET;
}
