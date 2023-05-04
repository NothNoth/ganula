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
  int start_ts;
  unsigned short * current;
  unsigned int  current_size;

  unsigned short * next;
  unsigned int  next_size;

  unsigned int sample_idx;

  unsigned short sampleA[MAX_SAMPLE_SIZE];
  unsigned short sampleB[MAX_SAMPLE_SIZE];
} voice_buffer_t;

unsigned short custom_wave[MAX_SAMPLE_SIZE];
unsigned int custom_wave_size;

unsigned short display_buffer[MAX_SAMPLE_SIZE];
unsigned int display_buffer_size;

#define MAX_VOICES 8

wave_t wave_form;
bool gsynth_running = true;
voice_buffer_t voices[MAX_VOICES];
adsr_t adsr;

int pitchToFrequency(int pitch);
void generate_sample();
void init_buffers();
void refresh_display_buffer();
float adsr_get_level(int ts, int release_ts, adsr_t *config);

void gsynth_setup() {
  analogWriteResolution(12);
  analogWrite(AUDIO_PIN, 0);
  wave_form = WAVE_SQUARE;
  memset(voices, 0x00, MAX_VOICES * sizeof(voice_buffer_t));
  gsynth_running = true;

  init_buffers();
  gsynth_select_wave(WAVE_SIN);
  adsr.a_ms = 500;
  adsr.d_ms = 10;
  adsr.s = 0.6;
  adsr.r_ms = 200;
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
    voices[i].current_size = 0;
  //And next to B
    voices[i].next = voices[i].sampleB;
    voices[i].next_size = 0;

    voices[i].flip_buffers = false;
  }
}


void flip_buffers(int voice_idx) {
  unsigned short * tmp = voices[voice_idx].current;
  voices[voice_idx].start_ts = millis();
  voices[voice_idx].current = voices[voice_idx].next;
  voices[voice_idx].current_size = voices[voice_idx].next_size;

  voices[voice_idx].next = tmp;
  voices[voice_idx].next_size = 0;

  voices[voice_idx].flip_buffers = false;

  refresh_display_buffer();
}
void dacoutput() {
  gsynth_gen();
}

int gsynth_gen() {
  int i;
  //Not in run mode? Don't play any sound.
  if (!gsynth_running) {
    return 0;
  }

  unsigned int merge = 0;
  int voices_playing = 0;
  int current_ts = millis();

  for (i = 0; i < MAX_VOICES; i++) {  

    //Voice is off?  
    if ((voices[i].current_size) == 0) {
      if (voices[i].flip_buffers == false) {
        continue;
      }
      //Not playing but buffer flip requested
      flip_buffers(i);
    }
  
    voices_playing ++;
    float adsr_level = adsr_get_level(current_ts - voices[i].start_ts, 0, &adsr);
    merge += voices[i].current[voices[i].sample_idx] * adsr_level;
    voices[i].sample_idx++;

    //Reach end of buffer
    if (voices[i].sample_idx >= voices[i].current_size) {
      voices[i].sample_idx = 0; //restart

      if (voices[i].flip_buffers == true) { //Eventually flip buffers for a new sample
        flip_buffers(i);
      }
    }
  }

  //No voices playing? Just output 0
  if (voices_playing == 0) {
    analogWrite(AUDIO_PIN, 0);
    return 0;
  }
  int out = (int)(((float)merge)/(float)voices_playing);
  analogWrite(AUDIO_PIN, out);
  return out;
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
    voices[voice_idx].next_size = 0;
    voices[voice_idx].flip_buffers = true;
    //char dbg[64];
    //snprintf(dbg, 64, "Turning voice %d off.", voice_idx);
    //debug_print(dbg);
    return;
  }

 
  //Generate sample depending on waveform
  switch (wave_form) {
    case WAVE_SQUARE:
    {
      voices[voice_idx].next_size = tone_generate_square(voices[voice_idx].next, voices[voice_idx].wave_frequency);
      debug_print("Switch to square.");
    }
    break;
    case WAVE_SAW:
    {
      voices[voice_idx].next_size = tone_generate_saw(voices[voice_idx].next, voices[voice_idx].wave_frequency);
      debug_print("Switch to saw.");
    }
    break;
    case WAVE_ISAW:
    {
      voices[voice_idx].next_size = tone_generate_isaw(voices[voice_idx].next, voices[voice_idx].wave_frequency);
      debug_print("Switch to isaw.");
    }
    break;
    case WAVE_SIN:
    {
      voices[voice_idx].next_size = tone_generate_sin(voices[voice_idx].next, voices[voice_idx].wave_frequency);
      debug_print("Switch to sin.");
    }
    break;
    case WAVE_TRIANGLE:
    {
      voices[voice_idx].next_size = tone_generate_triangle(voices[voice_idx].next, voices[voice_idx].wave_frequency);
      debug_print("Switch to triangle.");
    }
    break;
    case WAVE_CUSTOM:
    {
      voices[voice_idx].next_size = tone_generate_custom(voices[voice_idx].next, custom_wave, custom_wave_size, voices[voice_idx].wave_frequency);
      debug_print("Switch to Custom.");
    }
    break;
    case WAVE_MAX:
    break;
  }

  voices[voice_idx].flip_buffers = true;
}

void note_on(int channel, int pitch, int velocity) {
  if (gsynth_running == false) {
    return;
  }

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
      char dbg[64];
      snprintf(dbg, 64, "Voice %d stopping freq %dHz", voice_idx, wave_frequency);
      debug_print(dbg);
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


void refresh_display_buffer() {
  int voice_idx;
  bool got_sound = false;

  memset(display_buffer, 0x00, MAX_SAMPLE_SIZE * sizeof(short));
  display_buffer_size = 0;


  for (display_buffer_size = 0; display_buffer_size < MAX_SAMPLE_SIZE; display_buffer_size++) {
    int used_voices = 0;
    int merge = 0;

    for (voice_idx = 0; voice_idx < MAX_VOICES; voice_idx++) {
      //Voice not playing or end reached
      if ((voices[voice_idx].current_size == 0) || (display_buffer_size >= voices[voice_idx].current_size)) {
        continue;
      }
      used_voices++;
      merge += voices[voice_idx].current[display_buffer_size];
    }
    if (used_voices == 0) {
      break;
    }
    got_sound = true;
    display_buffer[display_buffer_size] = (unsigned short)(merge/(float)used_voices);
  }
  if (got_sound) {
    display_sample(display_buffer, display_buffer_size, 0);
  } else {
    display_nosample();
  }
}

void gsynth_set_adsr(int a, int d, float s, int r) {
  adsr.a_ms = a;
  adsr.d_ms = d;
  adsr.s = s;
  adsr.r_ms = r;
}

float adsr_get_level(int ts, int release_ts, adsr_t *config) {
  float level;

  if (ts <= config->a_ms) { // In attack section
  // y = a.x + b
  // b = 0
  // 1.0 = a . config.a_ms + 0
  
    level = (float)ts/(float)config->a_ms;
    //printf("Attack %d => %f\n", ts, level);
    return level;
  }

return 1.0; //FIXME
  if (ts <= config->a_ms + config->d_ms) { //In decay
    // y = a.x + b
    //x = a_ms -> y = 1.0
    //x = d_ms -> y = s

    // 1.0 = a . a_ms + b
    // s = a . d_ms + b
    //
    // 1.0 - s = a.a_ms - a.d_ms
    // =>  1.0 - s = a . (a_ms - d_ms)
    // a = (1.0 - s) / (a_ms - d_ms)
    // b = 1.0 - a . a_ms
    // b = 1.0 - (1.0 - s)*a_ms / (a_ms - d_ms)

    float a = ((1.0 - config->s)/((float) config->a_ms - (float)config->d_ms)) ;
    float b = 1.0 - (1.0 - config->s) * config->a_ms / ((float)config->a_ms - (float) config->d_ms);
    level = a * ts + b;
    printf("Decay %d => %f\n", ts, level);
 
    return level;
  }

  if (release_ts == 0) { /// In sustain
    printf("Sustain %d => %f\n", ts, config->s);

    return config->s;
  }

  //In release
  //y = a.x + b

  // x = release_ts => y = s
  // x = (release_ts + r_ts) => y = 0

  // s = a . release_ts + b
  // 0 = a . (release_ts + r_ts) + b

  // b = - a . (release_ts + r_ts)
  // s = a .(-r_ts)

  // a = -s/r_ts
  // b = s/r_ts   . (release_ts + r_ts)

  // a = - s/r_ts
  // b = s.(release_ts + r_ts) / r_ts

  float a = - config->s / config->r_ms;
  float b = config->s * ((float) release_ts + config->r_ms) / (float) config->r_ms;
  level = a * ts + b;

  printf("Release %d => %f\n", ts, level);

  return level;
}