#ifndef _CONTROLS_H_
#define _CONTROLS_H_

typedef void (*controls_cb_t)(int);


void controls_setup();
void controls_loop();
void controls_register_pot_cb(controls_cb_t cb);
void controls_register_bt1_cb(controls_cb_t cb);
void controls_register_bt2_cb(controls_cb_t cb);
#endif