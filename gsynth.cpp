#include "gsynth.h"
#include "setup.h"
#include "tone_generator.h"
#include <string.h>
#include <math.h>

#ifdef _GRANULA_TESTS_
  #include "granula_tests_stubs.h"
#else
  #include "Arduino.h"
  #include "display.h"
  #include "ntm.h"
#endif


typedef enum {
  BUFFER_A = 0,
  BUFFER_B = 1
 } buffer_t;

typedef struct {
  unsigned int wave_frequency;

  bool flip_buffers;
  unsigned short * current;
  unsigned int * current_size;
  unsigned short * next;
  unsigned int * next_size;

  unsigned int sample_idx;

  unsigned short sampleA[MAX_SAMPLE_SIZE];
  unsigned int sampleA_size;

  unsigned short sampleB[MAX_SAMPLE_SIZE];
  unsigned int sampleB_size;
} voice_buffer_t;

unsigned short custom_wave[MAX_SAMPLE_SIZE];
unsigned int custom_wave_size;

#define MAX_VOICES 8

wave_t wave_form;
bool gsynth_running = true;
voice_buffer_t voices[MAX_VOICES];

int pitchToFrequency(int pitch);
void generate_sample();
void init_buffers();

void gsynth_setup() {
  analogWriteResolution(12);
  wave_form = WAVE_SQUARE;
  memset(voices, 0x00, MAX_VOICES * sizeof(voice_buffer_t));
  gsynth_running = true;

  init_buffers();
  gsynth_select_wave(WAVE_SIN);
}

void gsynth_enable(bool run) {
  gsynth_running = run;
  init_buffers();
}

void gsynth_nextwave() {
  wave_form = wave_t(((int)wave_form +1)%(int)WAVE_MAX);
}

void init_buffers() {
  memset(voices, 0x00, MAX_VOICES * sizeof(voice_buffer_t));

  for (int i = 0; i < MAX_VOICES; i++) {
  //By default current points to A
    voices[i].current = voices[i].sampleA;
    voices[i].current_size = &voices[i].sampleA_size;
  //And next to B
    voices[i].next = voices[i].sampleB;
    voices[i].next_size = &voices[i].sampleB_size;

    voices[i].flip_buffers = false;
  }
}


void flip_buffers(int voice_idx) {
  unsigned short * tmp = voices[voice_idx].current;
  unsigned int * tmp_size = voices[voice_idx].current_size;

  voices[voice_idx].current = voices[voice_idx].next;
  voices[voice_idx].current_size = voices[voice_idx].next_size;

  voices[voice_idx].next = tmp;
  voices[voice_idx].next_size = tmp_size;

  voices[voice_idx].flip_buffers = false;
}
char dacdbg[128];

void dacoutput() {
  int i;
  //Not in run mode? Don't play any sound.
  if (!gsynth_running) {
    return;
  }

  int merge = 0;
  int voices_playing = 0;

  for (i = 0; i < MAX_VOICES; i++) {  

    //Voice is off?  
    if ((voices[i].wave_frequency) == 0) { //FIXME use *voices[i].current_size instead
      continue;
    }
    voices_playing ++;
    merge += voices[i].current[voices[i].sample_idx];
    voices[i].sample_idx++;
  sprintf(dacdbg, "DAC voice %d play idx %d", i, voices[i].sample_idx);

    //Reach end of buffer
    if (voices[i].sample_idx >= (*voices[i].current_size)) {
      voices[i].sample_idx = 0; //restart

      if (voices[i].flip_buffers == true) { //Eventually flip buffers for a new sample
        flip_buffers(i);
      }
    }
  }

  //No voices playing? Just output 0
  if (voices_playing == 0) {
    analogWrite(AUDIO_PIN, 0);
    return;
  }
  analogWrite(AUDIO_PIN, (int)(merge/(float)voices_playing));
}

void generate_sample(int voice_idx, int wave_frequency) {
  if (!gsynth_running) {
    debug_print("Won't generate sample: not running");
    return;
  }
  if (voice_idx >= MAX_VOICES) {
    debug_print("Won't generate sample: invalid voice idx");
    return;
  }
  voices[voice_idx].wave_frequency = wave_frequency;

  //Voice if off
  if (wave_frequency == 0) {
    (*voices[voice_idx].next_size) = 0;
    voices[voice_idx].flip_buffers = true;
    //char dbg[64];
    //snprintf(dbg, 64, "Turning voice %d off.", voice_idx);
    //debug_print(dbg);
    display_nosample();
    return;
  }

 
  //Generate sample depending on waveform
  switch (wave_form) {
    case WAVE_SQUARE:
    {
      *voices[voice_idx].next_size = tone_generate_square(voices[voice_idx].next, voices[voice_idx].wave_frequency);
      debug_print("Switch to square.");
    }
    break;
    case WAVE_SAW:
    {
      *voices[voice_idx].next_size = tone_generate_saw(voices[voice_idx].next, voices[voice_idx].wave_frequency);
      debug_print("Switch to saw.");
    }
    break;
    case WAVE_SIN:
    {
      *voices[voice_idx].next_size = tone_generate_sin(voices[voice_idx].next, voices[voice_idx].wave_frequency);
      debug_print("Switch to sin.");
    }
    break;
    case WAVE_TRIANGLE:
    {
      *voices[voice_idx].next_size = tone_generate_triangle(voices[voice_idx].next, voices[voice_idx].wave_frequency);
      debug_print("Switch to triangle.");
    }
    break;
    case WAVE_CUSTOM:
    {
      *voices[voice_idx].next_size = tone_generate_custom(voices[voice_idx].next, custom_wave, custom_wave_size, voices[voice_idx].wave_frequency);
      debug_print("Switch to Custom.");
    }
    break;
    case WAVE_MAX:
    break;
  }

  display_sample(voices[voice_idx].next, *voices[voice_idx].next_size, voices[voice_idx].wave_frequency);
  voices[voice_idx].flip_buffers = true;
}

void note_on(int channel, int pitch, int velocity) {
  if (gsynth_running == false) {
    return;
  }
  debug_print(dacdbg);

  int wave_frequency = pitchToFrequency(pitch);
  int voice_idx;

  //Find a free voice slot
  for (voice_idx = 0; voice_idx < MAX_VOICES; voice_idx++) {
    if (voices[voice_idx].wave_frequency == 0) {
      char dbg[64];
      sprintf(dbg, "Voice %d playing freq %dHz", voice_idx, wave_frequency);
      debug_print(dbg);
      generate_sample(voice_idx, wave_frequency);
      return;
    }
  }
}

void note_off(int channel, int pitch, int velocity) {
  int wave_frequency = pitchToFrequency(pitch);

  if (gsynth_running == false) {
    return;
  }

  int voice_idx;

  //Find matching current voice slot
  for (voice_idx = 0; voice_idx < MAX_VOICES; voice_idx++) {
    if (voices[voice_idx].wave_frequency == wave_frequency) {
      generate_sample(voice_idx, 0);
      //char dbg[64];
      //sprintf(dbg, "Voice %d stopping freq %dHz", voice_idx, wave_frequency);
      //debug_print(dbg);
      return;
    }

  }
  debug_print("Note off not found for freq");
}

int pitchToFrequency(int pitch) {
  return int(440.0 * pow(2.0, (pitch - 69.0) / 12.0));
}

void gsynth_save_custom(unsigned short *customrec, int len) {
  memcpy(custom_wave, customrec, (len>MAX_SAMPLE_SIZE?MAX_SAMPLE_SIZE:len)*sizeof(short));
  custom_wave_size = len;
  wave_form = WAVE_CUSTOM;
}

wave_t gsynth_getwaveforms(int idx) {
  if (idx >= int(WAVE_MAX)) {
    return WAVE_MAX;
  }

  return wave_t(idx);
}

void gsynth_select_wave(wave_t w) {
  if (w >= WAVE_MAX) {
    return;
  }
  wave_form = w;
  //generate_sample();
}
