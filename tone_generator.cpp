#include "tone_generator.h"
#include "setup.h"
#include "ntm.h"
/*

  TODO:

    - No-click:
    
    Fetch the last value of the current buffer and the first value of the new
    Create a one show buffer making a proper transition between the two
    



*/
unsigned int tone_generate_square(unsigned char*buffer, unsigned short frequency) {
  int i;

  //Since we're playing at given sample rate, we will be using sample_size space on the buffer
  unsigned int sample_size = SAMPLE_RATE / frequency;
  if (sample_size > MAX_SAMPLE_SIZE) {
    debug_print("sample buffer seems to small");
    return 0;
  }

  memset(buffer, 0x00, sample_size/2);
  memset(buffer+sample_size/2, 0xFF, sample_size/2);
  return sample_size;
}


unsigned int tone_generate_saw(unsigned char*buffer, unsigned short frequency) {
  int i;

  //Since we're playing at given sample rate, we will be using sample_size space on the buffer
  unsigned int sample_size = SAMPLE_RATE / frequency;
  if (sample_size > MAX_SAMPLE_SIZE) {
    debug_print("sample buffer seems to small");
    return 0;
  }

  memset(buffer, 0x00, sample_size);
  float increment = 255.0/(float)sample_size;
  for (i = 0; i < sample_size; i++) {
    buffer[i] = (unsigned char)((float)(i)*increment);
  }

  return sample_size;
}


unsigned int tone_generate_triangle(unsigned char*buffer, unsigned short frequency) {
  int i;

  //Since we're playing at given sample rate, we will be using sample_size space on the buffer
  unsigned int sample_size = SAMPLE_RATE / frequency;
  if (sample_size > MAX_SAMPLE_SIZE) {
    debug_print("sample buffer seems to small");
    return 0;
  }

  memset(buffer, 0x00, sample_size);
  float increment = 2*255.0/(float)sample_size;
  for (i = 0; i < sample_size/2; i++) {
    buffer[i] = (unsigned char)((float)(i)*increment);
  }
  for (i = 0; i < sample_size/2; i++) {
    buffer[i + sample_size/2] = (unsigned char)(255.0 - (float)(i)*increment);
  }
  return sample_size;
}

unsigned int tone_generate_sin(unsigned char*buffer, unsigned short frequency) {
  int i;

  //Since we're playing at given sample rate, we will be using sample_size space on the buffer
  unsigned int sample_size = SAMPLE_RATE / frequency;
  if (sample_size > MAX_SAMPLE_SIZE) {
    debug_print("sample buffer seems to small");
    return 0;
  }

  memset(buffer, 0x00, sample_size);
  for (i = 0; i < sample_size; i++) {
    buffer[i] = sin(i*2*3.1416/sample_size) * 128 + 128;//scale and center
  }

  return sample_size;
}


void tone_dump(unsigned char*buffer, unsigned short len) {
  int i;
  for (i = 0; i < len; i++) {
    debug_print(buffer[i]);
  }
}