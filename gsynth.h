#ifndef _GSYNTH_H_
#define _GSYNTH_H_


typedef enum {
  WAVE_SQUARE = 0,
  WAVE_SAW = 1,
  WAVE_SIN = 2,
  WAVE_TRIANGLE = 3,
  WAVE_CUSTOM = 4,
  WAVE_ISAW = 5,
  WAVE_MAX
 } wave_t;

typedef struct {
  int a_ms; //Attack in ms
  int d_ms; //Decay in ms
  float s;    //Sustain level [0.0-1.0]
  int r_ms; //Release is ms
} adsr_t;


void gsynth_setup();
void gsynth_enable(bool run);
void gsynth_nextwave();
void gsynth_set_adsr(int a, int d, float s, int r);
void gsynth_get_adsr(adsr_t *adsr);
void dacoutput();
void note_on(int channel, int pitch, int velocity);
void note_off(int channel, int pitch, int velocity);
void gsynth_save_custom(unsigned short *customrec, int len);


void gsynth_select_wave(wave_t w);

//For tests only
float adsr_get_level(int duration, int release_duration, adsr_t *config);

#endif