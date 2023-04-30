#include "gsynth.h"
#include "setup.h"
#include "tone_generator.h"
#include "Arduino.h"
#include "display.h"
#include "ntm.h"



typedef enum {
  BUFFER_A = 0,
  BUFFER_B = 1
 } buffer_t;

typedef struct {
  unsigned int wave_frequency;

  bool flip_buffers;
  unsigned char * current;
  unsigned int * current_size;
  unsigned char * next;
  unsigned int * next_size;

  unsigned int sample_idx;

  unsigned char sampleA[MAX_SAMPLE_SIZE];
  unsigned int sampleA_size;

  unsigned char sampleB[MAX_SAMPLE_SIZE];
  unsigned int sampleB_size;
} voice_buffer_t;

unsigned char custom_wave[MAX_SAMPLE_SIZE];
unsigned int custom_wave_size;

wave_t wave_form;
bool gsynth_running = true;
voice_buffer_t voice;
int flip_count;

int pitchToFrequency(int pitch);
void generate_sample();
void init_buffers();

void gsynth_setup() {
  wave_form = WAVE_SQUARE;
  voice.wave_frequency = 0;
  gsynth_running = true;

  init_buffers();
  generate_sample();
}

void gsynth_enable(bool run) {
  gsynth_running = run;
  init_buffers();

  generate_sample();
}

void gsynth_nextwave() {
  wave_form = wave_t(((int)wave_form +1)%(int)WAVE_MAX);
  generate_sample();
}

void init_buffers() {
  memset(&voice, 0x00, sizeof(voice_buffer_t));

//By default current points to A
  voice.current = voice.sampleA;
  voice.current_size = &voice.sampleA_size;
//And next to B
  voice.next = voice.sampleB;
  voice.next_size = &voice.sampleB_size;

  voice.flip_buffers = false;
}


void flip_buffers() {
  unsigned char * tmp = voice.current;
  unsigned int * tmp_size = voice.current_size;

  voice.current = voice.next;
  voice.current_size = voice.next_size;

  voice.next = tmp;
  voice.next_size = tmp_size;

  voice.flip_buffers = false;
  flip_count++;
}


void dacoutput() {
  //Not in run mode? Don't play any sound.
  if (!gsynth_running) {
    return;
  }

  //No buffer? Just output 0
  if (voice.current_size == 0) {
    analogWrite(AUDIO_PIN, 0);
    return;
  }

  analogWrite(AUDIO_PIN, voice.current[voice.sample_idx]);
  voice.sample_idx++;

  //Reach end of buffer
  if (voice.sample_idx >= (*voice.current_size)) {
    voice.sample_idx = 0; //restart

    if (voice.flip_buffers == true) { //Eventually flip buffers for a new sample
      flip_buffers();
    }
  }
}

void generate_sample() {
  unsigned int previous_last_value;
  if (!gsynth_running) {
    return;
  }

  previous_last_value = voice.current[(*voice.current_size)-1];

  //No sample played
  if (voice.wave_frequency == 0) {
    *voice.next_size = 0;
    voice.flip_buffers = true;
    display_nosample();
    return;
  }

 
  //Generate sample depending on waveform
  switch (wave_form) {
    case WAVE_SQUARE:
    {
      *voice.next_size = tone_generate_square(voice.next, voice.wave_frequency);
      debug_print("Switch to square.");
    }
    break;
    case WAVE_SAW:
    {
      *voice.next_size = tone_generate_saw(voice.next, voice.wave_frequency);
      debug_print("Switch to saw.");
    }
    break;
    case WAVE_SIN:
    {
      *voice.next_size = tone_generate_sin(voice.next, voice.wave_frequency);
      debug_print("Switch to sin.");
    }
    break;
    case WAVE_TRIANGLE:
    {
      *voice.next_size = tone_generate_triangle(voice.next, voice.wave_frequency);
      debug_print("Switch to triangle.");
    }
    break;
    case WAVE_CUSTOM:
    {
      *voice.next_size = tone_generate_custom(voice.next, custom_wave, custom_wave_size, voice.wave_frequency);
      debug_print("Switch to Custom.");
    }
    break;
  }

  display_sample(voice.next, *voice.next_size, voice.wave_frequency);
  voice.flip_buffers = true;
}


void note_on(int channel, int pitch, int velocity) {
  if (gsynth_running == false) {
    return;
  }

  voice.wave_frequency = pitchToFrequency(pitch);
  char dbg[64];
  sprintf(dbg, "Changing freq to %dHz", voice.wave_frequency);
  debug_print(dbg);
  generate_sample();
}


void note_off(int channel, int pitch, int velocity) {
  if (gsynth_running == false) {
    return;
  }
  voice.wave_frequency = 0;
  generate_sample();
}


int pitchToFrequency(int pitch) {
  return int(440.0 * pow(2.0, (pitch - 69.0) / 12.0));
}


void gsynth_save_custom(unsigned char *customrec, int len) {
  memcpy(custom_wave, customrec, len>MAX_SAMPLE_SIZE?MAX_SAMPLE_SIZE:len);
  custom_wave_size = len;
  wave_form = WAVE_CUSTOM;
  voice.wave_frequency = 100;
}
