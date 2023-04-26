#include "setup.h"
#include "controls.h"
#include "ntm.h"

unsigned int bt1_value;
unsigned int bt2_value;
unsigned int pot_value;
int refresh_ts;


controls_cb_t pot_cb;
controls_cb_t bt1_cb;
controls_cb_t bt2_cb;
#define REFRESH_INTERVAL_MS 200

void controls_setup() {
  pinMode(CONTROLS_BT1, INPUT);
  pinMode(CONTROLS_BT2, INPUT);
  pinMode(CONTROLS_POT, INPUT);

  bt1_value = analogRead(CONTROLS_BT1);
  bt2_value = analogRead(CONTROLS_BT2);
  pot_value = analogRead(CONTROLS_POT);
  refresh_ts = millis();
  pot_cb = NULL;
  bt1_cb = NULL;
  bt2_cb = NULL;
}


void controls_loop() {

  if (millis() - refresh_ts > REFRESH_INTERVAL_MS) {

    //Read pot value
    int value  = analogRead(CONTROLS_POT);
    if (value != pot_value) {
      pot_value = value;
      debug_print(pot_value);
      if (pot_cb) {
        pot_cb(pot_value);
      }
    }

    value  = analogRead(CONTROLS_BT1);
    if (value>>4 != bt1_value) {

      if (value>>4 > bt1_value) {
        debug_print("BT1 pressed");
        if (bt1_cb) {
          bt1_cb(0);
        }
      }

      bt1_value = value>>4;  
    }

    value  = analogRead(CONTROLS_BT2);
    if (value>>4 != bt2_value) {
      bt2_value = value>>4;
      debug_print("BT2 pressed");
      if (bt2_cb) {
        bt2_cb(0);
      }
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
