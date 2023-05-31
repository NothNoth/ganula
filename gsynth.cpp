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
  volatile int stop_ts;
  volatile unsigned int adsr_level;

  unsigned int  sample_size;
  unsigned int sample_idx;
  unsigned short sample[MAX_SAMPLE_SIZE];
} voice_buffer_t;

unsigned short custom_wave[MAX_SAMPLE_SIZE];
unsigned int custom_wave_size;




wave_t wave_form;
bool gsynth_running = true;
volatile voice_buffer_t voices[MAX_VOICES];
adsr_t adsr;
bool display_needs_refresh;

int pitchToFrequency(int pitch);
bool generate_sample(int voice_idx, int wave_frequency);
void init_voices();
void init_voice(int voice_idx);

void refresh_display_buffer();

void gsynth_setup() {
  analogWriteResolution(12);
  analogWrite(AUDIO_PIN, 0);
  wave_form = WAVE_SQUARE;
  memset((void*)voices, 0x00, MAX_VOICES * sizeof(voice_buffer_t));
  gsynth_running = true;

  init_voices();
  gsynth_select_wave(WAVE_SIN);
  gsynth_set_adsr(115, 200, 1020, 540);
  display_needs_refresh = true;
}

void gsynth_enable(bool run) {
  gsynth_running = run;
}

void gsynth_nextwave() {
  wave_form = wave_t(((int)wave_form +1)%(int)WAVE_MAX);
}

void init_voices() {
  memset((void*)voices, 0x00, MAX_VOICES * sizeof(voice_buffer_t));

  for (int i = 0; i < MAX_VOICES; i++) {
    init_voice(i);
  }
}

void init_voice(int voice_idx) {  
  voices[voice_idx].sample_size = 0;
  voices[voice_idx].free_voice = true;
}


void gsynth_loop() {
  int current_ts = millis();

  //Update ADSR coefficients
  for (int voice_idx = 0; voice_idx < MAX_VOICES; voice_idx++) {
    if (voices[voice_idx].free_voice == true) {
      continue;
    }

    if (voices[voice_idx].stop_ts == 0) {
      //Voice is still playing
      voices[voice_idx].adsr_level = adsr_get_level(current_ts - voices[voice_idx].start_ts, -1, &adsr);
    } else {
      //Voice is being released
      voices[voice_idx].adsr_level = adsr_get_level(current_ts - voices[voice_idx].start_ts, current_ts - voices[voice_idx].stop_ts, &adsr);

      if (voices[voice_idx].adsr_level == 0) {
        //Reached 0, time to free this voice
        init_voice(voice_idx);
        continue;
      }   
    }
    if  (voices[voice_idx].adsr_level > ADSR_RANGE) {
      voices[voice_idx].adsr_level = 0;
    }
  }

  if (display_needs_refresh) {
      refresh_display_buffer();
  }
}

void dacoutput() {
  int i;
  
  //Not in run mode? Don't play any sound.
  if (!gsynth_running) {
    return;
  }

  int merge = 0;
  int voices_playing = 0;
  unsigned int half_dac_range = MAX_DAC>>1;

  for (i = 0; i < MAX_VOICES; i++) {  
    volatile voice_buffer_t * voiceptr = &voices[i];
    //Voice is inactive, skip
    if (voiceptr->free_voice == true) {
      continue;
    }

    voices_playing ++;
    int value = voiceptr->sample[voiceptr->sample_idx];
    value -= half_dac_range;
    value = (value * voiceptr->adsr_level) >> ADSR_BITSHIFT;
    merge += value;
   
    voiceptr->sample_idx++;

    //Reach end of buffer
    if (voiceptr->sample_idx >= voiceptr->sample_size) {
      voiceptr->sample_idx = 0; //restart
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
  char dbg[64];
  snprintf(dbg, 64, "Voice %d playing freq %dHz", voice_idx, wave_frequency);
  dbg[63] = 0x00;

  switch (wave_form) {
    case WAVE_SQUARE:
    {
      voices[voice_idx].sample_size = tone_generate_square(voices[voice_idx].sample, wave_frequency);
      strcat(dbg, " (square)");
    }
    break;
    case WAVE_SAW:
    {
      voices[voice_idx].sample_size = tone_generate_saw(voices[voice_idx].sample, wave_frequency);
      strcat(dbg, " (saw)");
    }
    break;
    case WAVE_ISAW:
    {
      voices[voice_idx].sample_size = tone_generate_isaw(voices[voice_idx].sample, wave_frequency);
      strcat(dbg, " (isaw)");
    }
    break;
    case WAVE_SIN:
    {
      voices[voice_idx].sample_size = tone_generate_sin(voices[voice_idx].sample, wave_frequency);
      strcat(dbg, " (sin)");
    }
    break;
    case WAVE_TRIANGLE:
    {
      voices[voice_idx].sample_size = tone_generate_triangle(voices[voice_idx].sample, wave_frequency);
      strcat(dbg, " (triangle)");
    }
    break;
    case WAVE_CUSTOM:
    {
      voices[voice_idx].sample_size = tone_generate_custom(voices[voice_idx].sample, custom_wave, custom_wave_size, wave_frequency);
      strcat(dbg, " (custom)");
    }
    break;
    case WAVE_MAX:
    break;
  }
  debug_print(dbg);

  if (voices[voice_idx].sample_size == 0) {
    return false;
  }
  voices[voice_idx].wave_frequency = wave_frequency;
  voices[voice_idx].start_ts = millis();
  voices[voice_idx].stop_ts = 0;
  voices[voice_idx].free_voice = false;

  return true;
}

void note_on(int channel, int pitch, int velocity) {
  if (gsynth_running == false) {
    return;
  }
  int wave_frequency = pitchToFrequency(pitch);
  int voice_idx;
  int voice_free = -1;

  //Look for duplicates and find a free slot
  for (voice_idx = 0; voice_idx < MAX_VOICES; voice_idx++) {
    if ((voices[voice_idx].free_voice == false) && (voices[voice_idx].wave_frequency == wave_frequency)) {
      return;
    }
    if ((voice_free == -1) && (voices[voice_idx].free_voice == true)) {
      voice_free = voice_idx;
    }
  }

  //No more free slots
  if (voice_free == -1) {
    if (MAX_VOICES == 1) {
      //Violently release the only slot and re-use
      init_voice(0);
      voice_free = 0;
    } else {
      return;
    }
  }

  //Use a free voice slot
  generate_sample(voice_free, wave_frequency);
  display_needs_refresh = true;
}

void note_off(int channel, int pitch, int velocity) {
  int voice_idx;
  int wave_frequency = pitchToFrequency(pitch);

  if (gsynth_running == false) {
    return;
  }
  
  //Find matching current voice slot
  for (voice_idx = 0; voice_idx < MAX_VOICES; voice_idx++) {
    if ((voices[voice_idx].wave_frequency == wave_frequency) && (voices[voice_idx].stop_ts == 0)) {
      voices[voice_idx].stop_ts = millis(); //Mark as "releasing.."
      char dbg[64];
      snprintf(dbg, 64, "Voice %d stopping freq %dHz", voice_idx, wave_frequency);
      dbg[63] = 0x00;
      debug_print(dbg);
      display_needs_refresh = true;
      return;
    }
  }
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
  unsigned short display_buffer[MAX_SAMPLE_SIZE];
  unsigned int display_buffer_size;
  int work_buffer[MAX_SAMPLE_SIZE];
  unsigned int half_dac_range = MAX_DAC>>1;

  display_needs_refresh = false;
  memset(work_buffer, 0x00, MAX_SAMPLE_SIZE * sizeof(int));
  
  //Fetch longest voice sample
  display_buffer_size = 0;
  for (int voice_idx = 0; voice_idx < MAX_VOICES; voice_idx++) {
    if ((voices[voice_idx].free_voice == false) && (voices[voice_idx].stop_ts <= 0) && (voices[voice_idx].sample_size > display_buffer_size))
      display_buffer_size = voices[voice_idx].sample_size;
  }

  //Sum all voices
  int voices_playing = 0;
  for (int voice_idx = 0; voice_idx < MAX_VOICES; voice_idx++) {
    volatile voice_buffer_t * v = &voices[voice_idx];

    if ((v->free_voice == true) || (v->stop_ts > 0)) {
      continue; //Ignore that voice.
    }

    voices_playing++;
    for (int idx = 0; idx < display_buffer_size; idx++) {
      int val = v->sample[idx%v->sample_size];
      val -= half_dac_range;
      work_buffer[idx] += val;
    }
  }

  //Average and center
  for (int idx = 0; idx < display_buffer_size; idx++) {
    display_buffer[idx] = (unsigned short) (((float)work_buffer[idx] / (float)voices_playing) + half_dac_range);
  }

  //Not currently in RUN mode, don't disturb display.
  if (gmode_get() != PAGE_HOME) {
    return;
  }
  
  if (voices_playing != 0) {
    display_sample(display_buffer, display_buffer_size, 0);
  } else {
    display_nosample();
  }
}

void gsynth_set_adsr(int a, int d, int s, int r) {
  if (a < 0)
    a = 0;
  if (a > ADSR_MAX_ATTACK_MS)
    a = ADSR_MAX_ATTACK_MS;

  if (d < 0)
    d = 0;
  if (d > ADSR_MAX_DECAY_MS)
    d = ADSR_MAX_DECAY_MS;
  
  if ((s < 0) || (s >ADSR_RANGE))
    s = ADSR_RANGE;

  if (r < 0)
    r = 0;
  if (r > ADSR_MAX_RELEASE_MS)
    r = ADSR_MAX_RELEASE_MS;

  adsr.a_ms = a;
  adsr.d_ms = d;
  adsr.s = s;
  adsr.r_ms = r;

  char dbg[64];
  snprintf(dbg, 64, "Set adsr to: %dms/%dms/%d/%dms", adsr.a_ms, adsr.d_ms, adsr.s, adsr.r_ms);
  dbg[63] = 0x00;
  debug_print(dbg);
}

/*
  In an ideal world, ADSR level would be a value from 0.0 to 1.0, which
  multiplicated by the signal level would provide the required attenuation.
  Our Arduino doesn't have float support and since those computations are made
  inside the interrupt, we can't afford to waste time with software implementations.

  Thus:
  
    - The signal value ranges from 0 to MAX_DAC
    - The ADSR value ranges from 0 to 1024
    - When applied, out signal value is multiplicated by the ADSR value
    - Then divided by 1024 (<< 10)
*/
unsigned int adsr_get_level(int duration, int release_duration, adsr_t *config) {
  unsigned int level;

  if (duration <= config->a_ms) { // In attack section
    if (config->a_ms == 0) { // No attack, direct jump to max level
      return ADSR_RANGE;
    } 
    level = (int)(duration*ADSR_RANGE/(float)config->a_ms);
    return level<0?0:level;
  }

  if (duration <= config->a_ms + config->d_ms) { //In decay
    if (config->d_ms == 0) { // No decay, direct jump to sustain level
      return config->s;
    } 
    int a = (ADSR_RANGE - config->s) / ((float) (config->a_ms - config->d_ms));
    int b = config->s - a * config->d_ms;
    level = a * duration + b; 
    return level<0?0:level;
  }

  if (release_duration == -1) { /// In sustain
    return config->s;
  }

  //In release
  if (config->r_ms == 0) { // No release, direct jump to 0
    return 0;
  } 
  float a = -(((float)config->s / ((float)config->r_ms))) ;
  int b = config->s;
  level = a * release_duration + b;
  return level<0?0:level;
}

void gsynth_get_adsr(adsr_t *adsrptr) {
  memcpy(adsrptr, &adsr, sizeof(adsr_t));
}

