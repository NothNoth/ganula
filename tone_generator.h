#ifndef _TONE_GENERATOR_H_
#define _TONE_GENERATOR_H_

unsigned int tone_generate_square(volatile unsigned short*buffer, unsigned short frequency);
unsigned int tone_generate_saw(volatile unsigned short*buffer, unsigned short frequency);
unsigned int tone_generate_isaw(volatile unsigned short*buffer, unsigned short frequency);
unsigned int tone_generate_sin(volatile unsigned short*buffer, unsigned short frequency);
unsigned int tone_generate_triangle(volatile unsigned short*buffer, unsigned short frequency);
unsigned int tone_generate_custom(volatile unsigned short*buffer, unsigned short*custom_wave, unsigned int custom_wave_size, unsigned short wave_frequency);

#endif