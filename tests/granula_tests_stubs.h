#ifndef _DEBUG_H_
#define _DEBUG_H_
#include <stdio.h>

#define DAC1 12
#define OUTPUT 0
#define A0 0
#define A11 0
#define A10 0
#define A9 0
#define INPUT_PULLUP 0
#define INPUT 0
#define SSD1306_SWITCHCAPVCC 0
#define WHITE 0
#define BLACK 0


#define debug_setup(_BPS_) 
#define debug_print(_S_) printf("console> %s\n", _S_);

void debug_set_write_file(char *filename);
void debug_close_write_file();

void analogWriteResolution(int res);
void analogWrite(int port, int value);
int analogRead(int port);

int millis();
void delay(int d);


void pinMode(int a, int b);
class WireC {

};

class Adafruit_SSD1306 {
  public:
    Adafruit_SSD1306() {}
    Adafruit_SSD1306(int a, int b, WireC *w) {}
    void begin(int a, int b) {}
    void clearDisplay() {}
    void setCursor(int a, int b) {}
    void setTextSize(int a) {}
    void setTextColor(int a, int b) {}
    void setTextColor(int a) {}
    void drawPixel(int a, int b, int c) {}
    void display() {}
    void drawFastVLine(int a, int b, int c, int d) {}
    void drawFastHLine(int a, int b, int c, int d) {}
    void print(char*s) {}
    void invertDisplay(bool b) {}
};


static WireC Wire;


typedef struct {
  char header;
  char byte0;
  char byte1;
  char byte2;
  char byte3;
} midiEventPacket_t;


class MidiUsbC {
  public:
    midiEventPacket_t read() {midiEventPacket_t p; return p;}
};

static MidiUsbC MidiUSB;




typedef void (*daccb)(void);


class Whatever2 {
  public:
    void start() {}
};

class Whatever {
  public:
    Whatever2 setFrequency(int f) {Whatever2 w; return w;}
};

class Timer3C {
  public:
  Whatever attachInterrupt(daccb v) {Whatever w; return w;}
};

static Timer3C Timer3;





#endif