#ifndef _TONE_GENERATOR_H_
#define _TONE_GENERATOR_H_

unsigned int tone_generate_square(unsigned char*buffer, unsigned short frequency);
unsigned int tone_generate_saw(unsigned char*buffer, unsigned short frequency);
unsigned int tone_generate_sin(unsigned char*buffer, unsigned short frequency);
unsigned int tone_generate_triangle(unsigned char*buffer, unsigned short frequency);

void tone_dump(unsigned char*buffer, unsigned short len);

#endif