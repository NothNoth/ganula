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
void display_sample(unsigned char* sample, unsigned short len, unsigned int freq) {
  display.clearDisplay();
  int x = 0;
  int y;
#ifdef SHOW_TWO_PERIODS
  float xScale = (len/64.0);

  for (x = 0; x < 64; x++) {
    //From 255 range to 32 for Y axis => divide by 8 => 3 bits shift
    y = sample[int(x*xScale)]>>3;
    display.drawPixel(x, y, WHITE);
    display.drawPixel(x+64, y, WHITE);
    if (x+1 < 64) {
      antialias(x, y, sample[int((x+1)*xScale)]>>3);
      antialias(x+64, y, sample[int((x+1)*xScale)]>>3);
    }
  }
  antialias(x+64, sample[int(len-1)]>>3, sample[0]>>3);

#else
  float xScale = (len/128.0);
  for (x = 0; x < 128; x++) {
    //From 255 range to 32 for Y axis => divide by 8 => 3 bits shift
    y = sample[int(x*xScale)]>>3;
    display.drawPixel(x, y, WHITE);
    if (x+1 < 128) {
      antialias(x, y, sample[int((x+1)*xScale)]>>3);
    }
  }
#endif

  char txt[16];
  sprintf(txt,"%dHz", freq);
  display.setCursor(1,1);
  display.setTextSize(1);
  display.setTextColor(BLACK, WHITE);
  display.print(txt);

  display.display();
}

void display_nosample() {
  display.clearDisplay();
  display.drawFastHLine(0, 16, 128, WHITE);

  display.display();
}

//A dummy antialias which makes the link when there's a large
//vertical gap between two consecutive points (ie SAWTOOTH)
void antialias(int x, int y1, int y2) {
  if (abs(y2-y1) <= 1)
    return;

  display.drawFastVLine(x, y1<y2?y1:y2, y1<y2?y2-y1:y1-y2, WHITE);
}