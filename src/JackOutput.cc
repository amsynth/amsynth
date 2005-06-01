/* amSynth
 * (c) 2001-2004 Nick Dowell
 **/

#include "JackOutput.h"
#include "VoiceAllocationUnit.h"
#include <iostream>

#ifdef with_jack
VoiceAllocationUnit	*myinput;
float		*inbuf, *pt;
float		*lout, *rout;
jack_port_t 	*l_port, *r_port;
jack_client_t 	*client;
int		sample_rate;
int		buf_size;
int		p,q,initialised;


int
jack_process (jack_nframes_t nframes, void *arg)

{
	lout = (jack_default_audio_sample_t *)
			jack_port_get_buffer (l_port, nframes);
	rout = (jack_default_audio_sample_t *)
			jack_port_get_buffer (r_port, nframes);
	myinput->Process (lout, rout, nframes);
	return 0;
}

int
jack_bufsize (jack_nframes_t nframes, void *arg)
{
	buf_size = nframes;
	return 0;
}

int
jack_srate (jack_nframes_t nframes, void *arg)
{
	sample_rate = nframes;
	return 0;
}

void
jack_shutdown (void *arg)
{
}

void
jack_error	(const char* msg)
{
	std::cerr << msg << std::endl;
}

#endif

int
JackOutput::init	( Config & config )
{
#ifdef with_jack
	initialised = 0;
	client_name = "amSynth";
	
	// check if there are already any amSynth jack clients...
	
	const char **readports;
	
	if ( (client = jack_client_new( "amSynth-tmp" )) == 0 ) return -1;

	readports = jack_get_ports( client, NULL, NULL, JackPortIsOutput );
	
	int i=0, c=0;
	while (readports && readports[i])
	{
		if (strncmp( readports[i], "amSynth", 7 )==0) c++;
		i++;
	}
	c/=2;
	jack_client_close( client );
	if (c>0)
	{
		char tmp[3];
		sprintf( tmp, "%d", c );
		
		client_name += " (";
		client_name += string( tmp );
		client_name += ")";
	}
	
	/* become a client of the JACK server */
	if ((client = jack_client_new (client_name.c_str())) == 0)
	{
		error_msg = "jack_client_new() failed";
		return -1;
	}

	jack_set_process_callback (client, jack_process, 0);
	jack_set_buffer_size_callback (client, jack_bufsize, 0);
	jack_set_sample_rate_callback (client, jack_srate, 0);
	jack_on_shutdown (client, jack_shutdown, 0);

	sample_rate = jack_get_sample_rate( client );
	buf_size = p = jack_get_buffer_size( client );

	/* create output ports */
	l_port = jack_port_register( client, "L out", 
			JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0 );
	r_port = jack_port_register( client, "R out",
			JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0 );
	
	initialised = 1;
	
	config.sample_rate = sample_rate;
	config.buffer_size = buf_size;
	config.audio_driver = "JACK";
	
	return 0;
#endif
	return -1;
}


void
JackOutput::setInput( VoiceAllocationUnit* source )
{
#ifdef with_jack
	myinput = source;
#endif
	GenericOutput::setInput (source);
}

bool 
JackOutput::Start	()
{
#ifdef with_jack
	if (!initialised) return false;
	if (!myinput) return false;
	if (jack_activate (client)) 
	{
		std::cerr << "cannot activate JACK client\n";
		return false;
	}
	jack_connect(client, jack_port_name(l_port), "alsa_pcm:playback_1");
	jack_connect(client, jack_port_name(r_port), "alsa_pcm:playback_2");
	return true;
#else
	return false;
#endif
}

void
JackOutput::Stop()
{
#ifdef with_jack
	if (!initialised) return;
	jack_deactivate (client);
	jack_client_close (client);
#endif
}
