#ifndef _PAGE_SPLASH_H_
#define _PAGE_SPLASH_H_
#include "pages.hpp"


class PageSplash : public virtual Page{
  public:
    PageSplash();
    void enter();
    void leave();
    void loop();
    void button1_pressed();
    void button2_pressed();
    void pot_changed(int value);
};

#endif