/* amSynth
 * (c) 2001-2003 Nick Dowell
 */

#ifndef _JACK_OUTPUT_H
#define _JACK_OUTPUT_H

#ifdef with_jack
#include <jack/jack.h>
#endif

#include "AudioOutput.h"
#include "VoiceBoard/Synth--.h"
#include "Config.h"

class JackOutput : public GenericOutput {

public:
			JackOutput	( );
	virtual		~JackOutput	( ) {};
	void		setInput	( NFSource & source );
	
	void		run		( );
	void		stop		( );
	
	int		canRecord	( )		{ return 0; };
	void		startRecording	( );
	void		stopRecording	( );
	void		setOutputFile	( string file )	{ wavoutfile = file; };
	string		getOutputFile	( )		{ return wavoutfile; };

	void		setConfig	( Config & config );
	const char*	getTitle	( )	{ return client_name.c_str(); };

private:
	int	running;
	int	channels;
	Config	*config;
	string	wavoutfile;
	int	recording;
	int	bufsize, srate;
	string	client_name;
};

#endif				// _AUDIO_OUTPUT_H
