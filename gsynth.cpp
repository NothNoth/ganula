#include "gsynth.h"
#include "setup.h"
#include "tone_generator.h"
#include "Arduino.h"
#include "display.h"
#include "ntm.h"

unsigned char sample[MAX_SAMPLE_SIZE];
unsigned int sample_size;
unsigned int sample_idx;

wave_t wave_form;
unsigned int wave_frequency;
bool gsynth_running = true;


int pitchToFrequency(int pitch);


void gsynth_setup() {
  wave_form = WAVE_SQUARE;
  wave_frequency = 100;
  sample_idx = 0;
  sample_size = tone_generate_square(sample, 100);
  gsynth_running = true;
}

void gsynth_enable(bool run) {
  gsynth_running = run;
}

void gsynth_nextwave() {
  wave_form = wave_t(((int)wave_form +1)%(int)WAVE_MAX);
  refresh_sample();
}

void dacoutput() {
  //Not in run mode? Don't play any sound.
  if (!gsynth_running) {
    return;
  }
  analogWrite(AUDIO_PIN, sample[sample_idx]);
  sample_idx = (sample_idx+1)%sample_size;
}

void refresh_sample() {
  if (wave_frequency == 0) {
    sample_idx = 0;
    sample_size = 0;
    display_nosample();
    return;
  }

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


void note_on(int channel, int pitch, int velocity) {
  if (gsynth_running == false) {
    return;
  }

  wave_frequency = pitchToFrequency(pitch);
  char dbg[64];
  sprintf(dbg, "Changing freq to %dHz", wave_frequency);
  debug_print(dbg);
  refresh_sample();
}


void note_off(int channel, int pitch, int velocity) {
  if (gsynth_running == false) {
    return;
  }
  wave_frequency = 0;
  refresh_sample();
}


int pitchToFrequency(int pitch) {
  return int(440.0 * pow(2.0, (pitch - 69.0) / 12.0));
}

