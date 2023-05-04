#ifndef _DEBUG_H_
#define _DEBUG_H_
#include <stdio.h>

#define DAC1 12


#define debug_setup(_BPS_) 
#define debug_print(_S_) printf("%s\n", _S_);

#define analogWriteResolution(_resolution)
void analogWrite(int port, int value);

#define display_sample(_sample, _len, _freq)
#define display_nosample()

int millis();

#define gmode_get() 0
#endif