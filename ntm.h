#ifndef _DEBUG_H_
#define _DEBUG_H_
#include <Arduino.h>
#define debug_setup(_BPS_) Serial.begin(_BPS_)
#define debug_print(_S_) Serial.println(_S_)

#endif