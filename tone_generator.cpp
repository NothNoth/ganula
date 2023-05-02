#include "tone_generator.h"
#include "setup.h"
#ifdef _GRANULA_TESTS_
  #include "granula_tests_stubs.h"
#else
  #include "ntm.h"
#endif
#include <math.h>
#include <string.h>


void extrapolate(unsigned short *src, int src_size, unsigned short *dest, int dest_size);


unsigned int tone_generate_square(unsigned short*buffer, unsigned short frequency) {
  int i;

  //Since we're playing at given sample rate, we will be using sample_count space on the buffer
  unsigned int sample_count = SAMPLE_RATE / frequency;
  if (sample_count > MAX_SAMPLE_SIZE) {
    return 0;
  }

  int half = (int)(sample_count/2.0);
  memset(buffer, 0x00, half * sizeof(short));
  memset(buffer+half, 0xFF, half * sizeof(short));
  return sample_count;
}


unsigned int tone_generate_saw(unsigned short*buffer, unsigned short frequency) {
  int i;

  //Since we're playing at given sample rate, we will be using sample_count space on the buffer
  unsigned int sample_count = SAMPLE_RATE / frequency;
  if (sample_count > MAX_SAMPLE_SIZE) {
    return 0;
  }

  memset(buffer, 0x00, sample_count * sizeof(short));
  float increment = (float)(MAX_DAC)/(float)sample_count;
  for (i = 0; i < sample_count; i++) {
    buffer[i] = (unsigned short)((float)(i)*increment);
  }
  return sample_count;
}


unsigned int tone_generate_triangle(unsigned short*buffer, unsigned short frequency) {
  int i;

  //Since we're playing at given sample rate, we will be using sample_count space on the buffer
  unsigned int sample_count = SAMPLE_RATE / frequency;
  if (sample_count > MAX_SAMPLE_SIZE) {
    return 0;
  }

  memset(buffer, 0x00, sample_count * sizeof(short));
  float increment = 2*(float)(MAX_DAC)/(float)sample_count;
  for (i = 0; i < sample_count/2; i++) {
    buffer[i] = (unsigned short)((float)(i)*increment);
  }
  for (i = 0; i < sample_count/2; i++) {
    buffer[i + sample_count/2] = (unsigned short)((float)(MAX_DAC) - (float)(i)*increment);
  }
  return sample_count;
}

unsigned int tone_generate_sin(unsigned short*buffer, unsigned short frequency) {
  int i;

  //Since we're playing at given sample rate, we will be using sample_count space on the buffer
  unsigned int sample_count = SAMPLE_RATE / frequency;
  if (sample_count > MAX_SAMPLE_SIZE) {
    return 0;
  }

  memset(buffer, 0x00, sample_count*sizeof(short));
  int half_range = (int)(MAX_DAC/2.0);
  for (i = 0; i < sample_count; i++) {
    buffer[i] = sin(i*2*3.1416/sample_count) * half_range + half_range;//scale and center
  }

  return sample_count;
}

unsigned int tone_generate_custom(unsigned short*buffer, unsigned short*custom_wave, unsigned int custom_wave_size, unsigned short frequency) {
  int i;

  //Since we're playing at given sample rate, we will be using sample_count space on the buffer
  unsigned int sample_count = SAMPLE_RATE / frequency;
  if (sample_count > MAX_SAMPLE_SIZE) {
    return 0;
  }

  memset(buffer, 0x00, sample_count*sizeof(short));

  //Now map custom_wave of size custom_wave_size
  // to buffer of size sample_count
  extrapolate(custom_wave, custom_wave_size, buffer, sample_count);

  return sample_count;
}

void extrapolate(unsigned short *src, int src_size, unsigned short *dest, int dest_size) {
  int i;

  memset(dest, 0x00, dest_size*sizeof(short));

  if (dest_size > src_size) {
    float scale = dest_size/src_size;

    for (int i = 0; i < src_size; i++) {
      int idx = i*scale;
      for (int j = 0; j < scale; j++) {
        dest[idx+j] = src[i];
      }
    }
  } else {
    float scale = (float)src_size/(float)dest_size;
    for (i = 0; i < dest_size; i++) {
      int idx = (int)(scale * (float)(i));
      dest[i] = src[idx];
    }
/*
    float scale = (float)src_size/(float)dest_size;
    for (i = 0; i < dest_size; i++) {
      int start_idx = (int)(scale * (float)(i));
      int end_idx = (int)(scale * (float)(i+1));
      int tot = 0;
      for (int j = start_idx; j < end_idx; j++) {
        tot += src[j];
      }
      char plop[64];
      sprintf(plop, "%d: %d->%d / tot %d value: %d\n", i, start_idx, end_idx, tot, (int)((float)(tot)/(float)(end_idx - start_idx)));
      debug_print(plop);
      dest[i] = (int)((float)(tot)/(float)(end_idx - start_idx));
    }
    */
  }
}