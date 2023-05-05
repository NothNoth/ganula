#include "gsynth.h"
#include "setup.h"
#include "granula.h"
#include "tone_generator.h"
#include <string.h>
#include <math.h>
#include "display.h"

#ifdef _GRANULA_TESTS_
  #include "granula_tests_stubs.h"
#else
  #include "Arduino.h"
  #include "ntm.h"
#endif


typedef enum {
  BUFFER_A = 0,
  BUFFER_B = 1
 } buffer_t;

typedef struct {
  bool free_voice;

  unsigned int wave_frequency;
  int start_ts;
  int stop_ts;

  unsigned int  sample_size;
  unsigned int sample_idx;
  unsigned short sample[MAX_SAMPLE_SIZE];
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
bool generate_sample(int voice_idx, int wave_frequency);
void init_voices();
void init_voice(int voice_idx);

void refresh_display_buffer();
float adsr_get_level(int duration, int release_duration, adsr_t *config);

void gsynth_setup() {
  analogWriteResolution(12);
  analogWrite(AUDIO_PIN, 0);
  wave_form = WAVE_SQUARE;
  memset(voices, 0x00, MAX_VOICES * sizeof(voice_buffer_t));
  gsynth_running = true;

  init_voices();
  gsynth_select_wave(WAVE_SIN);
  adsr.a_ms = 500;
  adsr.d_ms = 100;
  adsr.s = 0.6;
  adsr.r_ms = 900;
}

void gsynth_enable(bool run) {
  gsynth_running = run;
}

void gsynth_nextwave() {
  wave_form = wave_t(((int)wave_form +1)%(int)WAVE_MAX);
}

void init_voices() {
  memset(voices, 0x00, MAX_VOICES * sizeof(voice_buffer_t));

  for (int i = 0; i < MAX_VOICES; i++) {
    init_voice(i);
  }
}
void init_voice(int voice_idx) {
  voices[voice_idx].sample_size = 0;
  voices[voice_idx].free_voice = true;
}

void dacoutput() {
  int i;
  //Not in run mode? Don't play any sound.
  if (!gsynth_running) {
    return;
  }

  float merge = 0.0;
  int voices_playing = 0;
  int current_ts = millis();
  float half_dac_range = ((float)MAX_DAC)/2.0;
  for (i = 0; i < MAX_VOICES; i++) {  
    float adsr_level;

    //Voice is inactive, skip
    if (voices[i].free_voice == true) {
      continue;
    }
  
    voices_playing ++;

    if (voices[i].stop_ts == 0) {
      //Voice is still playing
      adsr_level = adsr_get_level(current_ts - voices[i].start_ts, -1, &adsr);
    } else {
      //Voice is being released
      adsr_level = adsr_get_level(current_ts - voices[i].start_ts, current_ts - voices[i].stop_ts, &adsr);
      if (adsr_level < 0.001) {
        //Reached 0, time to free this voice
        init_voice(i);
        continue;
      }   
    }

    merge += (((float)voices[i].sample[voices[i].sample_idx]) - half_dac_range) * adsr_level ;
    voices[i].sample_idx++;

    //Reach end of buffer
    if (voices[i].sample_idx >= voices[i].sample_size) {
      voices[i].sample_idx = 0; //restart
    }
  }

  //No voices playing? Just output 0
  if (voices_playing == 0) {
    analogWrite(AUDIO_PIN, half_dac_range);
    return;
  }

  int out = (int)(((merge)/(float)voices_playing) + half_dac_range);
  analogWrite(AUDIO_PIN, out);
  return;
}

bool generate_sample(int voice_idx, int wave_frequency) {
  if (!gsynth_running) {
    debug_print("Won't generate sample: not running");
    return false;
  }
  if (voice_idx >= MAX_VOICES) {
    debug_print("Won't generate sample: invalid voice idx");
    return false;
  }
 
  //Generate sample depending on waveform
  switch (wave_form) {
    case WAVE_SQUARE:
    {
      voices[voice_idx].sample_size = tone_generate_square(voices[voice_idx].sample, wave_frequency);
      debug_print("Switch to square.");
    }
    break;
    case WAVE_SAW:
    {
      voices[voice_idx].sample_size = tone_generate_saw(voices[voice_idx].sample, wave_frequency);
      debug_print("Switch to saw.");
    }
    break;
    case WAVE_ISAW:
    {
      voices[voice_idx].sample_size = tone_generate_isaw(voices[voice_idx].sample, wave_frequency);
      debug_print("Switch to isaw.");
    }
    break;
    case WAVE_SIN:
    {
      voices[voice_idx].sample_size = tone_generate_sin(voices[voice_idx].sample, wave_frequency);
      debug_print("Switch to sin.");
    }
    break;
    case WAVE_TRIANGLE:
    {
      voices[voice_idx].sample_size = tone_generate_triangle(voices[voice_idx].sample, wave_frequency);
      debug_print("Switch to triangle.");
    }
    break;
    case WAVE_CUSTOM:
    {
      voices[voice_idx].sample_size = tone_generate_custom(voices[voice_idx].sample, custom_wave, custom_wave_size, wave_frequency);
      debug_print("Switch to Custom.");
    }
    break;
    case WAVE_MAX:
    break;
  }
  if (voices[voice_idx].sample_size == 0) {
    return false;
  }
  voices[voice_idx].wave_frequency = wave_frequency;
  voices[voice_idx].start_ts = millis();
  voices[voice_idx].stop_ts = 0;
  voices[voice_idx].free_voice = false;

  refresh_display_buffer();

  return true;
}

void note_on(int channel, int pitch, int velocity) {
  if (gsynth_running == false) {
    return;
  }

  int wave_frequency = pitchToFrequency(pitch);
  int voice_idx;
  
  //Look for duplicates
  for (voice_idx = 0; voice_idx < MAX_VOICES; voice_idx++) {
    if ((voices[voice_idx].free_voice == false) && (voices[voice_idx].wave_frequency == wave_frequency)) {
      return;
    }
  }

  //Find a free voice slot
  for (voice_idx = 0; voice_idx < MAX_VOICES; voice_idx++) {
    if (voices[voice_idx].free_voice == true) {
      char dbg[64];
      snprintf(dbg, 64, "Voice %d playing freq %dHz", voice_idx, wave_frequency);
      dbg[63] = 0x00;
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
    if ((voices[voice_idx].wave_frequency == wave_frequency) && (voices[voice_idx].stop_ts == 0)) {
      voices[voice_idx].stop_ts = millis(); //Mark as "releasing.."
      char dbg[64];
      snprintf(dbg, 64, "Voice %d stopping freq %dHz", voice_idx, wave_frequency);
      debug_print(dbg);
      return;
    }

  }
  //debug_print("Note off not found for freq");
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
      if ((voices[voice_idx].free_voice == true) || (display_buffer_size >= voices[voice_idx].sample_size)) {
        continue;
      }
      used_voices++;
      merge += voices[voice_idx].sample[display_buffer_size];
    }
    if (used_voices == 0) {
      break;
    }
    got_sound = true;
    display_buffer[display_buffer_size] = (unsigned short)(merge/(float)used_voices);
  }

//Not currently in RUN mode, don't disturb display.
  if (gmode_get() != GMODE_RUN) {
    return;
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

float adsr_get_level(int duration, int release_duration, adsr_t *config) {
  float level;

  if (duration <= config->a_ms) { // In attack section
    if (config->a_ms == 0) { // No attack, direct jump to max level
      return 1.0;
    } 
    level = (float)duration/(float)config->a_ms;
    return level<0.0?0.0:level;
  }

  if (duration <= config->a_ms + config->d_ms) { //In decay
    if (config->d_ms == 0) { // No decay, direct jump to sustain level
      return config->s;
    } 
    float a = (config->s - 1.0) / ((float) config->d_ms);
    float b = 1.0 - (((float)config->s - 1.0)/(float)config->d_ms) * (float)config->a_ms;
    level = a * duration + b; 
    return level<0.0?0.0:level;
  }

  if (release_duration == -1) { /// In sustain
    return config->s;
  }


  //In release
  if (config->r_ms == 0) { // No release, direct jump to 0.0
    return 0.0;
  } 
  float a = -((float)config->s) / ((float)config->r_ms) ;
  float b = (float) config->s;
  level = a * release_duration + b;

  return level<0.0?0.0:level;
}

void gsynth_get_adsr(adsr_t *adsrptr) {
  memcpy(adsrptr, &adsr, sizeof(adsr_t));
}

