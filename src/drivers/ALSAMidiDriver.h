/* Synth--
 * (c) 2001-2006 Nick Dowell
 **/

#ifndef _ALSA_MIDI_DRIVER_H
#define _ALSA_MIDI_DRIVER_H

#include "MidiDriver.h"

#include <iostream>
#ifdef with_alsa
#define ALSA_PCM_OLD_HW_PARAMS_API
#define ALSA_PCM_OLD_SW_PARAMS_API
#include <alsa/asoundlib.h>
#endif

class ALSAMidiDriver : public MidiDriver {
public:
		ALSAMidiDriver		( );
	virtual ~ALSAMidiDriver		( );
	int 	read			(unsigned char *bytes, unsigned maxBytes);
	int		write_cc		(unsigned int channel, unsigned int param, unsigned int value);
	int 	open			( Config & config );
	int 	close			( );
	int 	get_alsa_client_id	( )	{ return client_id; };
private:
#ifdef with_alsa
	snd_seq_t		*seq_handle;
	snd_midi_event_t	*seq_midi_parser;
	int 			portid;
	int				portid_out;
#endif
	int			client_id;
	int 			_bytes_read;
};

#endif
