#include <stdio.h>
#include <stdlib.h>
#include "granula_tests_stubs.h"
#include "../tone_generator.h"
#include "../setup.h"
#include "../gsynth.h"


#define FAIL(_str) { perror(_str); exit(-1);}

void test_tone_generator();
void test_gsynth();
void test_poly();
void test_adsr();

int main(int argc, char*argv[]) {
  printf("Granula - Tests\n");
  //test_tone_generator();
  //test_gsynth();
  //test_poly();

  test_adsr();
  return 0;
}

void test_tone_generator() {
  unsigned short buffer[MAX_SAMPLE_SIZE];

  printf("Test: square generation...\n");
  for (int i = 20; i < 8000; i+= 100) {
    unsigned int used = tone_generate_square(buffer, i);
    if (used > MAX_SAMPLE_SIZE) {
      FAIL("Used size too large.");
    }
  }

  printf("Test: sin generation...\n");
  for (int i = 20; i < 8000; i+= 100) {
    unsigned int used = tone_generate_sin(buffer, i);
    if (used > MAX_SAMPLE_SIZE) {
      FAIL("Used size too large.");
    }
  }

  printf("Test: triangle generation...\n");
  for (int i = 20; i < 8000; i+= 100) {
    unsigned int used = tone_generate_triangle(buffer, i);
    if (used > MAX_SAMPLE_SIZE) {
      FAIL("Used size too large.");
    }
  }

    printf("Test: saw generation...\n");
  for (int i = 20; i < 8000; i+= 100) {
    unsigned int used = tone_generate_saw(buffer, i);
    if (used > MAX_SAMPLE_SIZE) {
      FAIL("Used size too large.");
    }
  }

  printf("Test: custom generation...\n");
  unsigned short custom_small[16];
  for (int i = 0; i < 16; i++) {
    custom_small[i] = i * 12;
  }
  for (int i = 20; i < 8000; i+= 100) {
    unsigned int used = tone_generate_custom(buffer, custom_small, 16, i);
    if (used > MAX_SAMPLE_SIZE) {
      FAIL("Used size too large.");
    }
  }

  printf("Test: custom generation...\n");
  unsigned short custom_large[1024];
  for (int i = 0; i < 1024; i++) {
    custom_large[i] = i;
  }
  for (int i = 20; i < 8000; i+= 100) {
    unsigned int used = tone_generate_custom(buffer, custom_large, 1024, i);
    if (used > MAX_SAMPLE_SIZE) {
      FAIL("Used size too large.");
    }
  }

  return;
}


void test_gsynth() {
  printf("Test: gsynth setup...\n");
  gsynth_setup();
  gsynth_enable(true);

  printf("Test: gsynth dacoutput...\n");
  int pitch = 44;
  gsynth_select_wave(WAVE_SAW);

  for (int pitch = 0; pitch < 100; pitch++) {
    printf("Test: gsynth dacoutput (pitch %d)...\n", pitch);
    note_on(1, pitch, 120);
    for (int i = 0; i < 10000; i++) {
      dacoutput();
    }
    note_off(1, pitch, 0);
  }
}


void test_poly() {
  FILE *csv;
  gsynth_setup();
  gsynth_enable(true);

  printf("Test: gsynth dacoutput...\n");
  gsynth_select_wave(WAVE_SIN);

  note_on(1, 40, 120);

  csv = fopen("out.csv", "w+");
  for (int i = 0; i < 3000; i++) {
    if (i == 1000) {
      note_on(1, 60, 120);
    }

    if (i == 2000) {
      note_off(1, 40, 0);
    }
    if (i == 2800) {
      note_off(1, 60, 0);
    }
    fprintf(csv, "%d;%d\n", i, gsynth_gen());
  }
  fclose(csv);
}

#define EPSILON 0.0001

void test_adsr() {
  float level = 0.0;
  adsr_t adsr;

  adsr.a_ms = 100;
  adsr.d_ms = 50;
  adsr.s = 0.7;
  adsr.r_ms = 200;

printf("Attack starts\n");
  level = adsr_get_level(0, 0, &adsr);
  if (abs(level - 0.0) > EPSILON) {
    perror("Attack starts at level 0");
  }

printf("Attack ends\n");
  level = adsr_get_level(adsr.a_ms, 0, &adsr);
  if (abs(level - 1.0) > EPSILON) {
    printf("%f\n", level);
    perror("Attack ends at level 1.0");
  }

printf("Decay ends\n");
  level = adsr_get_level(adsr.a_ms + adsr.d_ms, 0, &adsr);
  if (abs(level - adsr.s) > EPSILON) {
    perror("Sustain ends at level 'sustain'");
  }

printf("Release starts\n");
  level = adsr_get_level(1000, 1, &adsr); //just released
  if (abs(level - adsr.s) > EPSILON) {
    perror("Release starts at level 'sustain'");
  }

printf("Release ends\n");
  level = adsr_get_level(1000, adsr.r_ms, &adsr); //fully released
  if (abs(level - 0.0) > EPSILON) {
    perror("Release ends at level 0.0");
  }
}