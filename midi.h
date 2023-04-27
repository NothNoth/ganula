#ifndef _MIDI_H_
#define _MIDI_H_

typedef void (*midi_noteon_cb_t)(int channel, int pitch, int velocity);
typedef void (*midi_noteoff_cb_t)(int channel, int pitch, int velocity);


void midi_setup();
void midi_loop();

void midi_register_noteon_cb(midi_noteon_cb_t cb);
void midi_register_noteoff_cb(midi_noteoff_cb_t cb);


#endif