#include "display.h"
#include "setup.h"
#include <math.h>
#ifdef _GRANULA_TESTS_
  #include "granula_tests_stubs.h"
#else
  #include "ntm.h"
  #include <SPI.h>
  #include <Wire.h>
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>
#endif

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

//Display a sample buffer on the LCD
void display_sample(unsigned short* sample, unsigned short len, unsigned int freq){
  int x = 0;
  int y;
  int xScale = len>>6; //Divide by 64
return; //FIXME
  display.clearDisplay();

  for (x = 0; x < 64; x++) {
    //From 4096 range to 32 for Y axis => divide by 128 => 7 bits shift
    y = sample[x*xScale]>>7;
    if (y >= 32) {
      y = 31;
    }
    display.drawPixel(x, y, WHITE);
    display.drawPixel(x+64, y, WHITE);
    if (x+1 < 64) {
      antialias(x, y, sample[(x+1)*xScale]>>7);
      antialias(x+64, y, sample[(x+1)*xScale]>>7);
    }
  }

  int y1 = sample[0]>>7;
  int y2 = sample[63 * xScale]>>7;
  for (y = (y1<y2?y1:y2); y < (y1<y2?y2:y1); y++) {
    display.drawPixel(64, y, WHITE);
  }

  if (freq != 0) {
    char txt[16];
    snprintf(txt, 16, "%dHz", freq);
    txt[15] = 0x00;
    display.setCursor(1,1);
    display.setTextSize(1);
    display.setTextColor(BLACK, WHITE);
    display.print(txt);
  }
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

void display_adsr(int a, int d, int s, int r, bool select_a, bool select_d, bool select_s, bool select_r) {
  int margin = 10;

  display.clearDisplay();
  display.setTextSize(1);

  display.setCursor(1, 3);
  display.setTextColor(select_a?BLACK:WHITE, select_a?WHITE:BLACK);
  display.print("A");
  display.drawFastHLine(margin, 6, (int)(a * (128.0 - margin)/1000.0), WHITE);

  display.setCursor(1, 10);
  display.setTextColor(select_d?BLACK:WHITE, select_d?WHITE:BLACK);
  display.print("D");
  display.drawFastHLine(margin, 13, (int)(d * (128.0 - margin)/1000.0), WHITE);

  display.setCursor(1, 18);
  display.setTextColor(select_s?BLACK:WHITE, select_s?WHITE:BLACK);
  display.print("S");
  display.drawFastHLine(margin, 21, (int)(s * (128.0 - margin)/100.0), WHITE);
 
  display.setCursor(1, 26);
  display.setTextColor(select_r?BLACK:WHITE, select_r?WHITE:BLACK);
  display.print("R");
  display.drawFastHLine(margin, 29, (int)(r * (128.0 - margin)/2000.0), WHITE);

  display.display();
}
