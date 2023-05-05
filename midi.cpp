#include "midi.h"
#ifdef _GRANULA_TESTS_
  #include "granula_tests_stubs.h"
#else
  #include "MIDIUSB.h"
  #include "ntm.h"
#endif

midi_noteon_cb_t noteon_cb;
midi_noteoff_cb_t noteoff_cb;

void midi_setup() {
  noteon_cb = NULL;
  noteoff_cb = NULL;
}

void midi_loop() {

  midiEventPacket_t rx;
  rx = MidiUSB.read();
  if (rx.header == 0) {
    return;
  }
    
/*
  Serial.print("Received: ");
  Serial.print(rx.header, HEX);
  Serial.print("-");
  Serial.print(rx.byte1, HEX);
  Serial.print("-");
  Serial.print(rx.byte2, HEX);
  Serial.print("-");
  Serial.println(rx.byte3, HEX);
*/
  if (((rx.header & 0x0F )== 0x09) && noteon_cb) { //Note on
    noteon_cb(int(rx.byte1&0xF), int(rx.byte2), int(rx.byte3));
  }
  else if (((rx.header & 0x0F) == 0x08) && noteoff_cb) { //Note off
    noteoff_cb(int(rx.byte1&0xF), int(rx.byte2), int(rx.byte3));
  }
}

void midi_register_noteon_cb(midi_noteon_cb_t cb) {
  noteon_cb = cb;
}

void midi_register_noteoff_cb(midi_noteoff_cb_t cb) {
  noteoff_cb = cb;
}
