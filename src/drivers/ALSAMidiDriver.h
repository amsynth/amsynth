/* Synth--
 * (c) 2001-2003 Nick Dowell
 **/

#ifndef _ALSA_MIDI_DRIVER_H
#define _ALSA_alsa_MIDI_DRIVER_H

#include "MidiDriver.h"

#include <iostream>
#ifdef with_alsa
#include <alsa/asoundlib.h>
#endif

class ALSAMidiDriver : public MidiDriver {
public:
    ALSAMidiDriver();
    virtual ~ ALSAMidiDriver();
    int read(unsigned char *midi_event_buffer);
    int open( string device, string name );
    int close();
private:
#ifdef with_alsa
	snd_seq_t *seq_handle;
	snd_midi_event_t *seq_midi_parser;
	int portid;
	int npfd;
	struct pollfd *pfd;
#endif
    int _bytes_read;
};

#endif				// with_alsa_MIDI_DRIVER_H
