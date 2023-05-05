#include "granula_tests_stubs.h"
#include <sys/times.h>

FILE * writeFile = NULL;

void debug_set_write_file(char *filename) {
  writeFile = fopen(filename, "wb+");
}

void debug_close_write_file() {
  fclose(writeFile);
  writeFile = NULL;
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


void pinMode(int a, int b) {

}

void delay(int d) {

}

int analogRead(int port) {
  return 0;
}

void analogWriteResolution(int res) {
  
}
