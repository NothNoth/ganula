#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "granula_tests_stubs.h"
#include "../tone_generator.h"
#include "../setup.h"
#include "../gsynth.h"


#define TEST_START(_str) printf("\n\n[Testing %s]...\n", _str);
#define TEST_END(_str)   printf("[Finished %s.]\n", _str);
#define TEST_SUB(_str)   printf("\t%s\n", _str);
#define TEST_ASSERT(_condition, _str) if ((_condition) == false) {printf("[FAILED] At line %d: %s\n", __LINE__, _str); exit(-1);} else {printf("[PASSED] %s\n", _str);}
#define FAIL(_str) { perror(_str); exit(-1);}


void test_tone_generator();
void test_gsynth();
void test_poly();
void test_adsr();
void test_fuzz();

enum {
  TONE   = 0x000001,
  GSYNTH = 0x000002,
  POLY = 0X4,
  ADSR = 0x8,
  FUZZ = 0x10
};

int main(int argc, char*argv[]) {
  unsigned int options = 0;

  printf("Granula - Functional Tests\n");

  if ((argc == 2) && (!strcmp(argv[1], "-h"))) {
    printf("Usage:\n");
    printf("%s [test target 1] ...\n\n", argv[0]);
    printf("Test targets:\n");
    printf("  TONE : test tone generator\n");
    printf("  GSYNTH : test synth\n");
    printf("  POLY : test polyphony\n");
    printf("  ADSR : test envelope generation\n");
    printf("  FUZZ : Fuzz the whole synth (never ends)\n");
    return 0;
  }

  for (int i  = 1; i < argc; i++) {
    if (!strcmp(argv[i], "TONE"))
      options |= TONE;
    if (!strcmp(argv[i], "GSYNTH"))
      options |= GSYNTH;
    if (!strcmp(argv[i], "POLY"))
      options |= POLY;
    if (!strcmp(argv[i], "ADSR"))
      options |= ADSR;
    if (!strcmp(argv[i], "FUZZ"))
      options |= FUZZ;
  }

  if ((options & TONE) == TONE)
    test_tone_generator();
  if ((options & GSYNTH) == GSYNTH)
    test_gsynth();
  if ((options & POLY) == POLY)
    test_poly();
  if ((options & ADSR) == ADSR)
    test_adsr();
  if ((options & FUZZ) == FUZZ)
    test_fuzz();

  return 0;
}

void test_tone_generator() {
  unsigned short buffer[MAX_SAMPLE_SIZE];
  TEST_START("TONE generation")

  TEST_SUB("Test: square generation...");
  for (int i = 20; i < 8000; i+= 100) {
    unsigned int used = tone_generate_square(buffer, i);
    TEST_ASSERT(used > 0, "Tone is properly generated");
    printf("Used %d i %d max %d\n", used, i, MAX_SAMPLE_SIZE);
    TEST_ASSERT(used < MAX_SAMPLE_SIZE, "Generated tone fits in sample buffer");
  }

  TEST_SUB("Test: sin generation...");
  for (int i = 20; i < 8000; i+= 100) {
    unsigned int used = tone_generate_sin(buffer, i);
    TEST_ASSERT(used > 0, "Tone is properly generated");
    TEST_ASSERT(used < MAX_SAMPLE_SIZE, "Generated tone fits in sample buffer");
  }

  TEST_SUB("Test: triangle generation...");
  for (int i = 20; i < 8000; i+= 100) {
    unsigned int used = tone_generate_triangle(buffer, i);
    TEST_ASSERT(used > 0, "Tone is properly generated");
    TEST_ASSERT(used < MAX_SAMPLE_SIZE, "Generated tone fits in sample buffer");
  }

  TEST_SUB("Test: saw generation...");
  for (int i = 20; i < 8000; i+= 100) {
    unsigned int used = tone_generate_saw(buffer, i);
    TEST_ASSERT(used > 0, "Tone is properly generated");
    TEST_ASSERT(used < MAX_SAMPLE_SIZE, "Generated tone fits in sample buffer");
  }

  TEST_SUB("Test: custom generation...");
  unsigned short custom_small[16];
  for (int i = 0; i < 16; i++) {
    custom_small[i] = i * 12;
  }
  for (int i = 20; i < 8000; i+= 100) {
    unsigned int used = tone_generate_custom(buffer, custom_small, 16, i);
    TEST_ASSERT(used > 0, "Tone is properly generated");
    TEST_ASSERT(used < MAX_SAMPLE_SIZE, "Generated tone fits in sample buffer");
  }

  TEST_SUB("Test: custom generation...");
  unsigned short custom_large[1024];
  for (int i = 0; i < 1024; i++) {
    custom_large[i] = i;
  }
  for (int i = 20; i < 8000; i+= 100) {
    unsigned int used = tone_generate_custom(buffer, custom_large, 1024, i);
    TEST_ASSERT(used > 0, "Tone is properly generated");
    TEST_ASSERT(used < MAX_SAMPLE_SIZE, "Generated tone fits in sample buffer");
  }
  TEST_END("TONE generation");
  return;
}


void test_gsynth() {
  TEST_START("GSYNTH");
  TEST_SUB("Test: gsynth setup...");
  debug_set_write_file("gsynth_test.csv");


  gsynth_setup();
  gsynth_set_adsr(0, 0, 1.0, 0);
  gsynth_enable(true);

  TEST_SUB("Test: gsynth dacoutput...");
  int pitch = 44;
  gsynth_select_wave(WAVE_SAW);

  for (int pitch = 0; pitch < 100; pitch++) {
    note_on(1, pitch, 120);
    for (int i = 0; i < 10000; i++) {
      dacoutput();
    }
    note_off(1, pitch, 0);
  }
  debug_close_write_file();
  TEST_END("GSYNTH");
}


void test_poly() {
  TEST_START("POLYphony");


  gsynth_setup();
  gsynth_enable(true);
  gsynth_set_adsr(0, 0, 1.0, 0);
  TEST_SUB("Test: gsynth dacoutput...");
  gsynth_select_wave(WAVE_SAW);

  note_on(1, 40, 120);

  debug_set_write_file("poly_test.csv");
  for (int i = 0; i < 10000; i++) {
    if (i == 2500) {
      note_on(1, 60, 120);
    }
    if (i == 5000) {
      note_off(1, 60, 0);
    }
    if (i == 7000) {
      note_off(1, 40, 0);
    }


    usleep(1000 * 1000.0/(float)SAMPLE_RATE);
    dacoutput();
  }

  debug_close_write_file();
  TEST_SUB("Manual check of poly_test.csv");
  TEST_END("POLYphony");
}

#define EPSILON 0.01

void test_adsr() {
  float level = 0.0;
  adsr_t adsr;

  TEST_START("ADSR");
  adsr.a_ms = 100;
  adsr.d_ms = 50;
  adsr.s = 70;
  adsr.r_ms = 200;

//Dump adsr curve
  FILE * f = fopen("adsr_test.csv", "wb+");
  for (int ts = 0; ts < 500; ts+=10) {
    fprintf(f, "%d;%d\n", ts, adsr_get_level(ts, ts<250?-1:ts-250, &adsr));
  }
  fclose(f);

//Test curve with a scenario
  level = adsr_get_level(0, 0, &adsr);
  TEST_ASSERT(level == 0, "Attack starts at level 0");

  level = adsr_get_level(adsr.a_ms, 0, &adsr);
  TEST_ASSERT(level == 100, "Attack ends at level 100");

  level = adsr_get_level(adsr.a_ms + adsr.d_ms, 0, &adsr);
  TEST_ASSERT(level == adsr.s, "Sustain ends at level 'sustain'");

  level = adsr_get_level(1000, 0, &adsr); //just released
  TEST_ASSERT(level == adsr.s, "Release starts at level 'sustain'");

  level = adsr_get_level(1000, adsr.r_ms, &adsr); //fully released
  TEST_ASSERT(level == 0, "Release ends at level 0");

  level = adsr_get_level(1000, 2*adsr.r_ms, &adsr); //very fully released
  TEST_ASSERT(level == 0, "After full release, level is still at 0");

  TEST_END("ADSR");
}


static void *dac(void*args){
  while (1) {
    dacoutput();
  }
  return NULL;
}

void test_fuzz() {
  TEST_START("FUZZing");
  gsynth_setup();
  gsynth_enable(true);

  TEST_SUB("Test: gsynth dacoutput...\n");
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
  TEST_END("FUZZing");
}
