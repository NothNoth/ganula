#ifndef _PAGES_H_
#define _PAGES_H_

class Page {
  public:
    virtual void enter() = 0;
    virtual void leave() = 0;
    virtual void loop() = 0;
    virtual void button1_pressed() = 0;
    virtual void button2_pressed() = 0;
    virtual void pot_changed(int value) = 0;
};


#endif
