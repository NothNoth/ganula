#include "display.h"
#include "ntm.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


Adafruit_SSD1306 display;

#define OLED_ADDRESS 0x3C
#define SHOW_TWO_PERIODS //Show two complete periods of the waveform?

void antialias(int x, int y1, int y2);

void display_setup() {
  display = Adafruit_SSD1306(128, 32, &Wire);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  display.clearDisplay();
  display.display();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("[Granula]");
  display.display();
}

void display_loop() {
}

//Display a sample buffer on the LCD (shows one or two periods depending on SHOW_TWO_PERIODS flag)
void display_sample(unsigned char* sample, unsigned short len) {
  display.clearDisplay();
  int x = 0;

#ifdef SHOW_TWO_PERIODS
  float xScale = (len/64.0);

  for (x = 0; x < 64; x++) {
    //From 255 range to 32 for Y axis => divide by 8 => 3 bits shift
    int y = sample[int(x*xScale)]>>3;
    display.drawPixel(x, y, WHITE);
    if (x+1 < 64) {
      antialias(x, y, sample[int((x+1)*xScale)]>>3);
    }
  }
  for (x = 0; x < 64; x++) {
    //From 255 range to 32 for Y axis => divide by 8 => 3 bits shift
    int y = sample[int((x+64)*xScale)]>>3;
    display.drawPixel(x+64, y, WHITE);
    if (x+1 < 64) {
      antialias(x + 64, y, sample[int(((x+64)+1)*xScale)]>>3);
    }
  }
#else
  float xScale = (len/128.0);
  for (x = 0; x < 128; x++) {
    //From 255 range to 32 for Y axis => divide by 8 => 3 bits shift
    int y = sample[int(x*xScale)]>>3;
    display.drawPixel(x, y, WHITE);
    if (x+1 < 128) {
      antialias(x, y, sample[int((x+1)*xScale)]>>3);
    }
  }
#endif
  display.display();
}

//A dummy untested antialias which makes the link when there's a large
//vertical gap between two consecutive points (ie SAWTOOTH)
void antialias(int x, int y1, int y2) {
  if (abs(y2-y1) <= 1)
    return;

  display.drawFastVLine(x, y1<y2?y1:y2, y1<y2?y2-y1:y1-y2, WHITE);
}