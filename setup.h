#ifndef _SETUP_H_
#define _SETUP_H_

//CPU frequency (used for timers)
#define CPU_FREQ 16000000

//Granula will read the output buffer at this sample rate
//and write to the DAC output accordingly.
#define SAMPLE_RATE 48000

//Audio output pin
#define AUDIO_PIN DAC1

//If playing a 20Hz tone at given SAMPLE RATE we would need a buffer of that size
#define MAX_SAMPLE_SIZE (SAMPLE_RATE/20)+1

//Maximum DAC output value
#define MAX_DAC 4096

//Potentiometer read value range
#define POT_RANGE 512

#define MAX_VOICES 8

#define ADSR_RANGE    1024
#define ADSR_BITSHIFT 10 // <-- divided by ADSR_RANGE gives
#define ADSR_MAX_ATTACK_MS 1000
#define ADSR_MAX_DECAY_MS 1000
#define ADSR_MAX_RELEASE_MS 2000


//Controls pins
#define CONTROLS_BT1 A9
#define CONTROLS_BT2 A10
#define CONTROLS_POT A11
#endif