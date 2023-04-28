#include "setup.h"
#include "controls.h"
#include "ntm.h"

unsigned int bt1_trigger;
unsigned int bt2_trigger;
unsigned int pot_value;
int refresh_ts;


controls_cb_t pot_cb;
controls_cb_t bt1_cb;
controls_cb_t bt2_cb;
#define REFRESH_INTERVAL_MS 100

void controls_setup() {
  pinMode(CONTROLS_BT1, INPUT_PULLUP);
  pinMode(CONTROLS_BT2, INPUT_PULLUP);
  pinMode(CONTROLS_POT, INPUT);

  bt1_trigger = 0;
  bt2_trigger = 0;
  pot_value = analogRead(CONTROLS_POT);
  refresh_ts = millis();
  pot_cb = NULL;
  bt1_cb = NULL;
  bt2_cb = NULL;
}


void controls_loop() {

  if (millis() - refresh_ts > REFRESH_INTERVAL_MS) {

    //Read pot value
    int value  = analogRead(CONTROLS_POT)>>1;
    if (value != pot_value) {
      pot_value = value;
      if (pot_cb) {
        pot_cb(pot_value);
      }
    }

    value  = analogRead(CONTROLS_BT1);
    if (value != 0) { //Button is not pressed
      if (bt1_trigger == 1) { //was pressed => just released
        bt1_trigger = 0;
        debug_print("BT1");
        if (bt1_cb) {
          bt1_cb(0);
        }
      }
    } else { //Button is pressed
      if (bt1_trigger == 0)
        bt1_trigger = 1;
    }


    value  = analogRead(CONTROLS_BT2);
    if (value != 0) { //Button is not pressed
      if (bt2_trigger == 1) { //was pressed => just released
        bt2_trigger = 0;
        debug_print("BT2");
        if (bt2_cb) {
          bt2_cb(0);
        }
      }
    } else { //Button is pressed
      if (bt2_trigger == 0)
        bt2_trigger = 1;
    }


    refresh_ts = millis();
  }
  
}

void controls_register_pot_cb(controls_cb_t cb) {
  pot_cb = cb;
}

void controls_register_bt1_cb(controls_cb_t cb) {
  bt1_cb = cb;
}

void controls_register_bt2_cb(controls_cb_t cb) {
  bt2_cb = cb;
}
