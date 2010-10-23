/* amSynth
 * (c) 2001-2010 Nick Dowell
 */

#ifndef _JACK_OUTPUT_H
#define _JACK_OUTPUT_H

#ifdef with_jack
#include <jack/jack.h>
#endif

#include "AudioOutput.h"
#include "Config.h"

class MidiStreamReceiver;

class JackOutput : public GenericOutput {

public:
	
	int			init		( Config & config );// returns 0 on success
	bool		Start		();
	void		Stop		();
	
	string		get_error_msg	( )		{ return error_msg; };
	
	bool		canRecord	( )		{ return false; };
	void		setOutputFile	( string file )	{ wavoutfile = file; };
	string		getOutputFile	( )		{ return wavoutfile; };

	void		setMidiHandler(MidiStreamReceiver *midiHandler) { _midiHandler = midiHandler; }

#ifdef with_jack
	static int process(jack_nframes_t nframes, void *arg);
#endif
private:
	int	running;
	string	wavoutfile;
	int	recording;
	string	error_msg;
	bool _auto_connect;
#ifdef with_jack
	jack_port_t 	*l_port, *r_port, *m_port;
	jack_client_t 	*client;
#endif
	MidiStreamReceiver *_midiHandler;
};

#endif				// _JACK_OUTPUT_H
