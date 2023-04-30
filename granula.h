#ifndef _GRANULA_H_
#define _GRANULA_H_

typedef enum {
  GMODE_RUN = 0,
  GMODE_CUSTOM_POTSYNC = 1,
  GMODE_CUSTOM_REC = 2,
  GMODE_MAX
} gmode_t;

void gmode_switch(gmode_t new_mode);


#endif