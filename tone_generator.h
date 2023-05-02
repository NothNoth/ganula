#ifndef _TONE_GENERATOR_H_
#define _TONE_GENERATOR_H_

unsigned int tone_generate_square(unsigned short*buffer, unsigned short frequency);
unsigned int tone_generate_saw(unsigned short*buffer, unsigned short frequency);
unsigned int tone_generate_isaw(unsigned short*buffer, unsigned short frequency);
unsigned int tone_generate_sin(unsigned short*buffer, unsigned short frequency);
unsigned int tone_generate_triangle(unsigned short*buffer, unsigned short frequency);
unsigned int tone_generate_custom(unsigned short*buffer, unsigned short*custom_wave, unsigned int custom_wave_size, unsigned short wave_frequency);

#endif