#ifndef _DISPLAY_H_
#define _DISPLAY_H_

void display_setup();
void display_loop();

void display_sample(unsigned short* sample, unsigned short len, unsigned int freq);
void display_nosample();

void display_potsync(int potpos);

void display_rec_countdown();
void display_rec(int rec_bar, unsigned short potvalue, unsigned short *recsample);

void display_clear();
void display_text(char *text, int line, bool highlight);

void display_adsr(int a, int d, int s, int r, bool select_a, bool select_d, bool select_s, bool select_r);
#endif
