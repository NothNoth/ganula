#include "setup.h"
#include "ntm.h"
#include "tone_generator.h"
#include "controls.h"
#include "display.h"
#include <DueTimer.h>
#include "midi.h"

typedef enum {
  WAVE_SQUARE = 0,
  WAVE_SAW = 1,
  WAVE_SIN = 2,
  WAVE_TRIANGLE = 3,
  WAVE_MAX
 } wave_t;

unsigned char sample[MAX_SAMPLE_SIZE];
unsigned int sample_size;
unsigned int sample_idx;

wave_t wave_form;
unsigned int wave_frequency;

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

  sample_idx = 0;

  sample_size = tone_generate_square(sample, 100);
  pinMode(AUDIO_PIN, OUTPUT);

  Timer3.attachInterrupt(dacoutput).setFrequency(SAMPLE_RATE).start();
  wave_form = WAVE_SQUARE;
  wave_frequency = 100;
}

void loop() {
  controls_loop();
  display_loop();
  midi_loop();
}


void dacoutput() {
  analogWrite(AUDIO_PIN, sample[sample_idx]);
  sample_idx = (sample_idx+1)%sample_size;
}


void pot_changed(int value) {
  value *= 2;
  if (value < 20) {
    value = 20;
  }
  wave_frequency = value;
  char dbg[64];
  sprintf(dbg, "Changing freq to %dHz", value);
  debug_print(dbg);

  refresh_sample();
}

void refresh_sample() {
switch (wave_form) {
    case WAVE_SQUARE:
    {
      sample_size = tone_generate_square(sample, wave_frequency);
      debug_print("Switch to square.");
    }
    break;
    case WAVE_SAW:
    {
      sample_size = tone_generate_saw(sample, wave_frequency);
      debug_print("Switch to saw.");
    }
    break;
    case WAVE_SIN:
    {
      sample_size = tone_generate_sin(sample, wave_frequency);
      debug_print("Switch to sin.");
    }
    break;
    case WAVE_TRIANGLE:
    {
      sample_size = tone_generate_triangle(sample, wave_frequency);
      debug_print("Switch to triangle.");
    }
    break;
  }
  display_sample(sample, sample_size, wave_frequency);
}

void bt1_pressed(int unused) {
  wave_form = wave_t(((int)wave_form +1)%(int)WAVE_MAX);
  refresh_sample();
}

void bt2_pressed(int unused) {
  
}

void note_on(int channel, int pitch, int velocity) {
  wave_frequency = pitchToFrequency(pitch);
  char dbg[64];
  sprintf(dbg, "Changing freq to %dHz", wave_frequency);
  debug_print(dbg);

  refresh_sample();
}


void note_off(int channel, int pitch, int velocity) {

}


int pitchToFrequency(int pitch) {
  return int(440.0 * pow(2.0, (pitch - 69.0) / 12.0));
}
