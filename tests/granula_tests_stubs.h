#ifndef _DEBUG_H_
#define _DEBUG_H_
#include <stdio.h>

#define DAC1 12


#define debug_setup(_BPS_) 
#define debug_print(_S_) printf("%s\n", _S_);

#define analogWriteResolution(_resolution)
#define analogWrite(_port,_value)

#define display_sample(_sample, _len, _freq)
#define display_nosample()


#endif