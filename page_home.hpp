#ifndef _PAGE_HOME_H_
#define _PAGE_HOME_H_
#include "pages.hpp"


class PageHome : public virtual Page{
  public:
    PageHome();
    void enter();
    void leave();
    void loop();
    void button1_pressed();
    void button2_pressed();
    void pot_changed(int value);
};

#endif