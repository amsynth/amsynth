/* Synth--
 * (c) 2001,2002 Nick Dowell
 **/

#ifndef _ALSA_MIDI_DRIVER_H
#define _ALSA_MIDI_DRIVER_H

#include "MidiDriver.h"

#include <iostream.h>
#ifdef _ALSA
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
#ifdef _ALSA
	snd_seq_t *seq_handle;
	snd_midi_event_t *seq_midi_parser;
	int portid;
	int npfd;
	struct pollfd *pfd;
#endif
    int _bytes_read;
};

#endif				// _ALSA_MIDI_DRIVER_H
