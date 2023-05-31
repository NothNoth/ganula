#ifndef _GRANULA_H_
#define _GRANULA_H_

typedef enum {
  PAGE_SPLASH,
  PAGE_HOME,
  PAGE_MENU,
  PAGE_REC,
  PAGE_ADSR
} gmode_t;

void gmode_switch(gmode_t new_mode);
gmode_t gmode_get();
void reset();

#endif