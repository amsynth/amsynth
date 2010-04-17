/* amSynth
 * (c) 2001-2005 Nick Dowell
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
				JackOutput	( );
	virtual		~JackOutput	( ) {};
	
	int			init		( Config & config );// returns 0 on success
	bool		Start		();
	void		Stop		();
	
	string		get_error_msg	( )		{ return error_msg; };
	
	bool		canRecord	( )		{ return false; };
	void		setOutputFile	( string file )	{ wavoutfile = file; };
	string		getOutputFile	( )		{ return wavoutfile; };

	const char*	getTitle	( )	{ return client_name.c_str(); };

#ifdef with_jack
	int			process		(jack_nframes_t nframes);
	int			bufsize		(jack_nframes_t nframes);
	int			srate		(jack_nframes_t nframes);
	void		shutdown	();
#endif
private:
	int	running;
	int	channels;
	string	wavoutfile;
	int	recording;
	string	client_name, error_msg;
#ifdef with_jack
	jack_port_t 	*l_port, *r_port;
	jack_client_t 	*client;
#endif
	int		sample_rate;
	int		buf_size;
	bool	initialised;
};

#endif				// _JACK_OUTPUT_H
