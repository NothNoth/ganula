#include "display.h"
#include "ntm.h"
#include "setup.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


Adafruit_SSD1306 display;

#define OLED_ADDRESS 0x3C
void antialias(int x, int y1, int y2);
void splash();

void display_setup() {
  display = Adafruit_SSD1306(128, 32, &Wire);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS);
  splash();
}

void splash() {
  display.clearDisplay();

  display.setCursor(1,8);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.print(" [Granula] ");
  display.invertDisplay(true);
  display.display();
  delay(1500);
  display.invertDisplay(false);
  display.clearDisplay();
  display.display();
}

void display_loop() {
}

//Display a sample buffer on the LCD (shows one or two periods depending on SHOW_TWO_PERIODS flag)
void display_sample(unsigned short* sample, unsigned short len, unsigned int freq) {
  display.clearDisplay();
  int x = 0;
  int y;
  float xScale = (len/64.0);


  for (x = 0; x < 64; x++) {
    //From 4096 range to 32 for Y axis => divide by 128 => 7 bits shift
    y = sample[int(x*xScale)]>>7;
    if (y >= 32) {
      y = 31;
    }
    display.drawPixel(x, y, WHITE);
    display.drawPixel(x+64, y, WHITE);
    if (x+1 < 64) {
      antialias(x, y, sample[int((x+1)*xScale)]>>7);
      antialias(x+64, y, sample[int((x+1)*xScale)]>>7);
    }
  }

  int y1 = sample[0 ]>>7;
  int y2 = sample[int(63 * xScale)]>>7;
  for (y = (y1<y2?y1:y2); y < (y1<y2?y2:y1); y++) {
    display.drawPixel(64, y, WHITE);
  }

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

void display_potsync(int potpos) {
  int i;

  int cursor = (potpos *128.0) / POT_RANGE;

  display.clearDisplay();

  for (i = 0; i < 32; i++) {
    display.drawPixel(63, i, WHITE);
    display.drawPixel(65, i, WHITE);
  }


  for (i = 0; i < 32; i++) {
    display.drawPixel(cursor, i, WHITE);
  }

  display.setCursor(1,1);
  display.setTextSize(1);
  display.setTextColor(BLACK, WHITE);
  display.print("Align bars");

  display.display();
}

void display_rec(int rec_bar, unsigned short potvalue, unsigned short *recsample) {
  int x, y;
  int xlmargin = 10;
  int xrmargin = 117;

  display.clearDisplay();
  
  //Print l r margins
  for (y = 0; y < 32; y+= 3) {
    display.drawPixel(xlmargin, y, WHITE);
    display.drawPixel(xrmargin, y, WHITE);
  }

  //Print mid
  for (y = 0; y < 32; y+= 5) {
    display.drawPixel(64, y, WHITE);
  }


  //Print 0 reference
  for (x = 0; x < 128; x+=2) {
    display.drawPixel(x, 16, WHITE);
  }

  //Print rec cursor
  for (y = 0; y < 32; y++) {
    display.drawPixel(rec_bar, y, WHITE);
  }

  //Print pot position
  // MAX_DAC -> 32
  // potvalue
  int crosspos = potvalue * 32/(float)MAX_DAC;

  display.drawPixel(rec_bar-1, crosspos-1, WHITE);
  display.drawPixel(rec_bar-1, crosspos, WHITE);
  display.drawPixel(rec_bar-1, crosspos+1, WHITE);

  display.drawPixel(rec_bar+1, crosspos-1, WHITE);
  display.drawPixel(rec_bar+1, crosspos, WHITE);
  display.drawPixel(rec_bar+1, crosspos+1, WHITE);

  display.drawPixel(rec_bar, crosspos-1, WHITE);
  display.drawPixel(rec_bar, crosspos, BLACK);
  display.drawPixel(rec_bar, crosspos+1, WHITE);

  //Print past
  int ends = rec_bar<xrmargin?rec_bar:xrmargin;
  for (x = xlmargin; x < ends; x++) {
    display.drawPixel(x, recsample[x]*32/(float)MAX_DAC, WHITE);
     //TODO: fill right margin with early data to help for smooth transitions
  }

  display.display();
}

void display_clear() {
  display.clearDisplay();
  display.display();
}

void display_text(char *text, int line, bool highlight) {

  display.setCursor(1, line*12);
  display.setTextSize(1);
  if (highlight) {
    display.setTextColor(BLACK, WHITE);
  } else {
    display.setTextColor(WHITE, BLACK);
  }
  display.print(text);
  display.display();
}