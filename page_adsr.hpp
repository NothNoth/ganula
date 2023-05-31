#ifndef _PAGE_ADSR_H_
#define _PAGE_ADSR_H_
#include "pages.hpp"


class PageADSR : public virtual Page{
  public:
    PageADSR();
    void enter();
    void leave();
    void loop();
    void button1_pressed();
    void button2_pressed();
    void pot_changed(int value);
  private:
    int adsr_select_idx;

};

#endif