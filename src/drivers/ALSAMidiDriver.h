/* Synth--
 * (c) 2001 Nick Dowell
 **/

#ifndef _ALSA_MIDI_DRIVER_H
#define _ALSA_MIDI_DRIVER_H

#include "MidiDriver.h"

#include <iostream.h>
#ifdef _ALSA
#include <alsa/asoundlib.h>
#endif

class ALSAMidiDriver:public MidiDriver {
  public:
    ALSAMidiDriver();
    virtual ~ ALSAMidiDriver();
    int read(unsigned char *midi_event_buffer);
    int open( string device );
    int close();
  private:
#ifdef _ALSA
     snd_rawmidi_t * _alsa_midi_handle;	// Handle for ALSA midi device
#endif
    int _alsa_midi_card;	// card # for ALSA driver
    int _alsa_midi_device;	// MIDI device # for ALSA driver
    int _bytes_read;
};

#endif				// _ALSA_MIDI_DRIVER_H
