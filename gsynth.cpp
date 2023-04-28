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
void generate_sample();


void gsynth_setup() {
  wave_form = WAVE_SQUARE;
  wave_frequency = 0;
  gsynth_running = true;
  generate_sample();
}

void gsynth_enable(bool run) {
  gsynth_running = run;
  generate_sample();
}

void gsynth_nextwave() {
  wave_form = wave_t(((int)wave_form +1)%(int)WAVE_MAX);
  generate_sample();
}

void dacoutput() {
  //Not in run mode? Don't play any sound.
  if (!gsynth_running) {
    return;
  }
  analogWrite(AUDIO_PIN, sample[sample_idx]);
  sample_idx = (sample_idx+1)%sample_size;
}

void generate_sample() {
  unsigned int previous_last_value;
  if (!gsynth_running) {
    return;
  }

  //No sample played
  if (wave_frequency == 0) {
    sample_idx = 0;
    sample_size = 0;
    display_nosample();
    return;
  }

  //Save where the previous wave ends
  previous_last_value = sample[sample_size-1];

  //Generate sample depending on waveform
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

  //Anti click trick:
  //Previous sample was ending at walue "previous_last_value"
  //-> find in new buffer an index where we have the same value
  // for a smooth transition (if none, start at index 0)
  sample_idx = 0;
  for (int i = 0; i < sample_size; i++) {
    if (sample[i] == previous_last_value) {
      sample_idx = i;
      break;
    }
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
  generate_sample();
}


void note_off(int channel, int pitch, int velocity) {
  if (gsynth_running == false) {
    return;
  }
  wave_frequency = 0;
  generate_sample();
}


int pitchToFrequency(int pitch) {
  return int(440.0 * pow(2.0, (pitch - 69.0) / 12.0));
}

