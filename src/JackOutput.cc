/* amSynth
 * (c) 2001-2010 Nick Dowell
 **/

#include "JackOutput.h"
#include "VoiceAllocationUnit.h"
#include "drivers/MidiInterface.h" // for class MidiStreamReceiver

#if HAVE_CONFIG_H
#include "config.h"
#endif

#if HAVE_JACK_MIDIPORT_H
#include <jack/midiport.h>
#endif

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

	l_port = r_port = m_port = 0;

	client = jack_client_open("amsynth", JackNoStartServer, NULL);
	if (!client) {
		error_msg = "jack_client_open() failed";
		return -1;
	}
	
	jack_set_process_callback(client, &JackOutput::process, this);

	/* create output ports */
	l_port = jack_port_register(client, "L out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	r_port = jack_port_register(client, "R out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

#if HAVE_JACK_MIDIPORT_H
	/* create midi input port(s) */
	m_port = jack_port_register(client, "midi_in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
#endif
	
	config.current_audio_driver = "JACK";
	config.sample_rate = jack_get_sample_rate(client);
	config.buffer_size = jack_get_buffer_size(client);
	config.jack_client_name = std::string(jack_get_client_name(client));

	if (m_port)
		config.current_midi_driver = "JACK";
	
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
#if HAVE_JACK_MIDIPORT_H
	if (self->m_port) {
		void *port_buf = jack_port_get_buffer(self->m_port, nframes);
		const jack_nframes_t event_count = jack_midi_get_event_count(port_buf);
		for (jack_nframes_t i=0; i<event_count; i++) {
			jack_midi_event_t midi_event;
			memset(&midi_event, 0, sizeof(midi_event));
			jack_midi_event_get(&midi_event, port_buf, i);
			if (midi_event.size && midi_event.buffer) {
				// TODO: use midi_event.time to reduce midi event jitter
				if (self->_midiHandler)
					self->_midiHandler->HandleMidiData(midi_event.buffer, midi_event.size);
			}
		}
	}
#endif
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
