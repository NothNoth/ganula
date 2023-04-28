#ifndef _DISPLAY_H_
#define _DISPLAY_H_

void display_setup();
void display_loop();
void display_sample(unsigned char* sample, unsigned short len, unsigned int freq);
void display_nosample();

#endif