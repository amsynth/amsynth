/* amSynth
 * (c) 2001-2010 Nick Dowell
 **/

#include "JackOutput.h"
#include "VoiceAllocationUnit.h"

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int
JackOutput::init	( Config & config )
{
#ifdef with_jack
	if (client) // already initialised
		return 0;

	client = jack_client_open("amsynth", JackNoStartServer, NULL);
	if (!client) {
		error_msg = "jack_client_open() failed";
		return -1;
	}
	
	jack_set_process_callback(client, &JackOutput::process, this);

	/* create output ports */
	l_port = jack_port_register(client, "L out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	r_port = jack_port_register(client, "R out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	
	config.current_audio_driver = "JACK";
	config.sample_rate = jack_get_sample_rate(client);
	config.buffer_size = jack_get_buffer_size(client);
	config.jack_client_name = std::string(jack_get_client_name(client));
	
	return 0;
#endif
	UNUSED_PARAM(config);
	return -1;
}

#ifdef with_jack
int
JackOutput::process (jack_nframes_t nframes, void *arg)
{
	JackOutput *self = (JackOutput *)arg;
	float *lout = (jack_default_audio_sample_t *) jack_port_get_buffer(self->l_port, nframes);
	float *rout = (jack_default_audio_sample_t *) jack_port_get_buffer(self->r_port, nframes);
	self->mInput->Process (lout, rout, nframes);
	return 0;
}
#endif

bool 
JackOutput::Start	()
{
#ifdef with_jack
	if (!client) return false;
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
	if (!client) return;
	jack_deactivate(client);
	jack_client_close(client);
	client = 0;
#endif
}
