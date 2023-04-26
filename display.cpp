#include "display.h"
#include "ntm.h"
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


Adafruit_SSD1306 display;

void display_setup() {

  
  display = Adafruit_SSD1306(128, 32, &Wire);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  display.clearDisplay();
  display.display();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.println("[Granula]");
  display.display();
}

void display_loop() {
}

void display_sample(unsigned char* sample, unsigned short len) {
  display.clearDisplay();
  float xScale = (len/128.0);
  int x = 0;

  for (x = 0; x < 128; x++) {
    //From 255 to 32 for Y axis => divide by 8 => 3 bits shift
    display.drawPixel(x, sample[int(x*xScale)]>>3, WHITE);
  }

  display.display();
}