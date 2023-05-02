#include <stdio.h>
#include <stdlib.h>
#include "granula_tests_stubs.h"
#include "../tone_generator.h"
#include "../setup.h"
#include "../gsynth.h"


#define FAIL(_str) { perror(_str); exit(-1);}

void test_tone_generator();
void test_gsynth();

int main(int argc, char*argv[]) {
  printf("Granula - Tests\n");

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

}


void test_gsynth() {
  gsynth_setup();
  gsynth_enable(true);
  dacoutput();
}
