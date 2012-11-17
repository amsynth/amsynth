/* amSynth
 * (c) 2001-2010 Nick Dowell
 */

#ifndef _JACK_OUTPUT_H
#define _JACK_OUTPUT_H

#if HAVE_CONFIG_H
#include "../config.h"
#endif

#ifdef WITH_JACK
#include <jack/jack.h>
#endif

#include "AudioOutput.h"
#include "Config.h"

class MidiStreamReceiver;

class JackOutput : public GenericOutput {

public:

	JackOutput();
	
	int			init		( Config & config );// returns 0 on success
	bool		Start		();
	void		Stop		();
	
	string		get_error_msg	( )		{ return error_msg; };
	
	void		setMidiHandler(MidiStreamReceiver *midiHandler) { _midiHandler = midiHandler; }

#ifdef WITH_JACK
	static int process(jack_nframes_t nframes, void *arg);
#endif
private:
	string	error_msg;
	bool _auto_connect;
#ifdef WITH_JACK
	jack_port_t 	*l_port, *r_port, *m_port;
	jack_client_t 	*client;
#endif
	MidiStreamReceiver *_midiHandler;
};

#endif				// _JACK_OUTPUT_H
