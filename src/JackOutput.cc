/* amSynth
 * (c) 2001-2007 Nick Dowell
 **/

#include "JackOutput.h"
#include "VoiceAllocationUnit.h"

#include <iostream>
#include <string.h>
#include <stdlib.h>

#define SIZEOF_ARRAY(a) (sizeof(a) / sizeof(a[0]))

#ifdef with_jack


int
jack_process (jack_nframes_t nframes, void *arg)
{
	JackOutput *client = (JackOutput *) arg;
	return client->process (nframes);
}

int
jack_bufsize (jack_nframes_t nframes, void *arg)
{
	JackOutput *client = (JackOutput *) arg;
	return client->bufsize (nframes);
}

int
jack_srate (jack_nframes_t nframes, void *arg)
{
	JackOutput *client = (JackOutput *) arg;
	return client->srate (nframes);
}

void
jack_shutdown (void *arg)
{
	JackOutput *client = (JackOutput *) arg;
	client->shutdown ();
}

void
jack_error	(const char* msg)
{
	std::cerr << msg << std::endl;
}

#endif

JackOutput::JackOutput()
{
	initialised = false;
	client_name = "amSynth";	
}

int
JackOutput::init	( Config & config )
{
#ifdef with_jack
	if (initialised)
		return 0;
	
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
	jack_client_close(client);
	if (c>0)
	{
		char tmp[3];
		sprintf( tmp, "%d", c );
		
		client_name += " (";
		client_name += string( tmp );
		client_name += ")";
	}
	
	/* become a client of the JACK server */
	if ((client = jack_client_new(client_name.c_str())) == 0)
	{
		error_msg = "jack_client_new() failed";
		return -1;
	}

	jack_set_process_callback(client, jack_process, this);
	jack_set_buffer_size_callback(client, jack_bufsize, this);
	jack_set_sample_rate_callback(client, jack_srate, this);
	jack_on_shutdown(client, jack_shutdown, this);

	sample_rate = jack_get_sample_rate( client );
	buf_size = jack_get_buffer_size(client);

	/* create output ports */
	l_port = jack_port_register(client, "L out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	r_port = jack_port_register(client, "R out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	
	initialised = true;
	
	config.sample_rate = sample_rate;
	config.buffer_size = buf_size;
	config.current_audio_driver = "JACK";
	
	return 0;
#endif
	UNUSED_PARAM(config);
	return -1;
}

#ifdef with_jack
int
JackOutput::process (jack_nframes_t nframes)
{
	float *lout = (jack_default_audio_sample_t *) jack_port_get_buffer(l_port, nframes);
	float *rout = (jack_default_audio_sample_t *) jack_port_get_buffer(r_port, nframes);
	mInput->Process (lout, rout, nframes);
	return 0;
}

int
JackOutput::bufsize (jack_nframes_t nframes)
{
	buf_size = nframes;
	return 0;
}

int
JackOutput::srate (jack_nframes_t nframes)
{
	sample_rate = nframes;
	return 0;
}

void
JackOutput::shutdown ()
{
}
#endif

bool 
JackOutput::Start	()
{
#ifdef with_jack
	if (!initialised) return false;
	if (!mInput) return false;
	if (jack_activate(client)) 
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
	jack_deactivate(client);
	jack_client_close(client);
#endif
}
