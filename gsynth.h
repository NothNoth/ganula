#ifndef _GSYNTH_H_
#define _GSYNTH_H_


typedef enum {
  WAVE_SQUARE = 0,
  WAVE_SAW = 1,
  WAVE_SIN = 2,
  WAVE_TRIANGLE = 3,
  WAVE_CUSTOM = 4,
  WAVE_MAX
 } wave_t;


void gsynth_setup();
void gsynth_enable(bool run);
void gsynth_nextwave();
void dacoutput();
void note_on(int channel, int pitch, int velocity);
void note_off(int channel, int pitch, int velocity);
void gsynth_save_custom(unsigned short *customrec, int len);


void gsynth_select_wave(wave_t w);

#endif