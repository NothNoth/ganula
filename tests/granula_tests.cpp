#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <sys/times.h>
#include "granula_tests_stubs.h"
#include "../tone_generator.h"
#include "../setup.h"
#include "../gsynth.h"


#define FAIL(_str) { perror(_str); exit(-1);}
FILE * writeFile = NULL;


void test_tone_generator();
void test_gsynth();
void test_poly();
void test_adsr();
void test_fuzz();

int main(int argc, char*argv[]) {
  printf("Granula - Functional Tests\n");

  writeFile = NULL;
  test_tone_generator();
  test_gsynth();
  test_poly();
  test_adsr();
  test_fuzz();

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
  writeFile = fopen("gsynth_test.csv", "wb+");


  gsynth_setup();
  gsynth_set_adsr(0, 0, 1.0, 0);
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
  fclose(writeFile);
  writeFile = NULL;
}


void test_poly() {
  gsynth_setup();
  gsynth_enable(true);

  printf("Test: gsynth dacoutput...\n");
  gsynth_select_wave(WAVE_SIN);

  note_on(1, 40, 120);

  writeFile = fopen("poly_test.csv", "w+");
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
    dacoutput();
  }
  fclose(writeFile);
  writeFile = NULL;
}

#define EPSILON 0.0001

void test_adsr() {
  float level = 0.0;
  adsr_t adsr;

  adsr.a_ms = 100;
  adsr.d_ms = 50;
  adsr.s = 0.7;
  adsr.r_ms = 200;

//Dump adsr curve
  FILE * f = fopen("adsr_test.csv", "wb+");
  for (int ts = 0; ts < 500; ts+=10) {
    fprintf(f, "%d;%f\n", ts, adsr_get_level(ts, ts<250?-1:ts-250, &adsr));
  }
  fclose(f);

//Test curve with a scenario
  level = adsr_get_level(0, 0, &adsr);
  if (abs(level - 0.0) > EPSILON) {
    perror("Attack starts at level 0");
  }

  level = adsr_get_level(adsr.a_ms, 0, &adsr);
  if (abs(level - 1.0) > EPSILON) {
    printf("%f\n", level);
    perror("Attack ends at level 1.0");
  }

  level = adsr_get_level(adsr.a_ms + adsr.d_ms, 0, &adsr);
  if (abs(level - adsr.s) > EPSILON) {
    perror("Sustain ends at level 'sustain'");
  }

  level = adsr_get_level(1000, 1, &adsr); //just released
  if (abs(level - adsr.s) > EPSILON) {
    printf("%f\n");
    perror("Release starts at level 'sustain'");
  }

  level = adsr_get_level(1000, adsr.r_ms, &adsr); //fully released
  if (abs(level - 0.0) > EPSILON) {
    perror("Release ends at level 0.0");
  }
}


static void *dac(void*args){
  while (1) {
    dacoutput();
  }
  return NULL;
}

void test_fuzz() {
  gsynth_setup();
  gsynth_enable(true);

  printf("Test: gsynth dacoutput...\n");
  gsynth_select_wave(WAVE_SIN);

  pthread_attr_t attr;
  pthread_t tid;
  memset(&attr, 0x00, sizeof(pthread_attr_t));
  pthread_attr_init(&attr);

  pthread_create(&tid, &attr, dac, NULL);

  while (1) {
    int r = rand()%100;

    if (r < 10) {
      int pitch = rand()%50 + 20;
      note_on(1, pitch, 100);
    } else if (r < 20) {
      int pitch = rand()%50 + 20;
      note_off(1, pitch, 0);
    } else if (r < 25) {
      gsynth_nextwave();
    } else {
      usleep(1000);
    }
  }
}
void analogWrite(int port, int value) {
  if (writeFile == NULL) {
    return;
  }

  fprintf(writeFile, "%d\n", value);
}

int millis() {
  struct tms tp;
  long t = times(&tp);

  return (int)(t&0xFFFFFF) * 10;
}
