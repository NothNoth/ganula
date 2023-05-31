#ifndef _PAGE_REC_H_
#define _PAGE_REC_H_
#include "pages.hpp"


#define CUSTOM_REC_SAMPLE_SIZE 128
#define CUSTOM_REC_SAMPLE_POINT_MS 120
#define CUSTOM_REC_SAMPLE_POT_MARGIN 128


class PageRec : public virtual Page{
  public:
    PageRec();
    void enter();
    void leave();
    void loop();
    void button1_pressed();
    void button2_pressed();
    void pot_changed(int value);
  private:
    int custom_rec_idx;
    int custom_rec_ts;
    unsigned short customrecsample[CUSTOM_REC_SAMPLE_SIZE];
    int last_pot_value;
    bool pot_sync;


};

#endif