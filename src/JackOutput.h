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

class JackOutput : public GenericOutput {

public:
	
	int			init		( Config & config );// returns 0 on success
	bool		Start		();
	void		Stop		();
	
	string		get_error_msg	( )		{ return error_msg; };
	
	bool		canRecord	( )		{ return false; };
	void		setOutputFile	( string file )	{ wavoutfile = file; };
	string		getOutputFile	( )		{ return wavoutfile; };

#ifdef with_jack
	static int process(jack_nframes_t nframes, void *arg);
#endif
private:
	int	running;
	string	wavoutfile;
	int	recording;
	string	error_msg;
#ifdef with_jack
	jack_port_t 	*l_port, *r_port;
	jack_client_t 	*client;
#endif
};

#endif				// _JACK_OUTPUT_H
