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
		ALSAMidiDriver		( );
	virtual ~ALSAMidiDriver		( );
	int 	read			( unsigned char *midi_event_buffer );
	int 	open			( Config & config );
	int 	close			( );
	int 	get_alsa_client_id	( )	{ return client_id; };
private:
#ifdef with_alsa
	snd_seq_t		*seq_handle;
	snd_midi_event_t	*seq_midi_parser;
	int 			portid;
#endif
	int			client_id;
	int 			_bytes_read;
};

#endif
