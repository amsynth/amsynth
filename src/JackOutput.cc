/* amSynth
 * (c) 2001-2010 Nick Dowell
 **/

#include "JackOutput.h"
#include "VoiceAllocationUnit.h"
#include "drivers/MidiInterface.h" // for class MidiStreamReceiver

#if HAVE_JACK_MIDIPORT_H
#include <jack/midiport.h>
#endif

#ifdef HAVE_JACK_SESSION_H
#include <jack/session.h>
#endif

#include <iostream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef HAVE_JACK_SESSION_H
static void session_callback(jack_session_event_t *event, void *arg);
#endif

JackOutput::JackOutput()
:	running(false)
#ifdef WITH_JACK
,	client(NULL)
#endif
{
}

int
JackOutput::init	( Config & config )
{
#ifdef WITH_JACK
	if (client) // already initialised
		return 0;

	l_port = r_port = m_port = 0;

	jack_status_t status = (jack_status_t)0;

#if HAVE_JACK_SESSION_H
	if (!config.jack_session_uuid.empty())
		client = jack_client_open("amysnth", JackSessionID,
				&status, config.jack_session_uuid.c_str());
	else
#endif
		client = jack_client_open("amsynth", JackNoStartServer, &status);
	if (!client) {
		std::ostringstream o;
		o << "jack_client_open() failed, status = 0x" << std::hex << status;
		error_msg = o.str();
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
	
#if HAVE_JACK_SESSION_H
	jack_set_session_callback(client, session_callback, client);
#endif

	config.current_audio_driver = "JACK";
	config.sample_rate = jack_get_sample_rate(client);
	config.buffer_size = jack_get_buffer_size(client);
	config.jack_client_name = std::string(jack_get_client_name(client));

	// don't auto connect ports if under jack session control...
	// the jack session manager is responsible for restoring port connections
	_auto_connect = config.jack_session_uuid.empty();
	
	return 0;
#endif
	UNUSED_PARAM(config);
	return -1;
}

#ifdef WITH_JACK
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
#ifdef WITH_JACK
	if (!client) return false;
	if (!mInput) return false;
	if (jack_activate(client)) 
	{
		std::cerr << "cannot activate JACK client\n";
		return false;
	}
	if (_auto_connect) {
		const char **port_names = jack_get_ports(client, NULL, NULL, JackPortIsPhysical | JackPortIsInput);
		if (port_names) {
			jack_connect(client, jack_port_name(l_port), port_names[0]);
			jack_connect(client, jack_port_name(r_port), port_names[1]);
		}
	}
	return true;
#else
	return false;
#endif
}

void
JackOutput::Stop()
{
#ifdef WITH_JACK
	if (!client) return;
	jack_deactivate(client);
	jack_client_close(client);
	client = 0;
#endif
}

#ifdef HAVE_JACK_SESSION_H

static void session_callback(jack_session_event_t *event, void *arg)
{
	char filename[1024]; snprintf(filename, sizeof(filename),
		"%s%s.amsynth.bank", event->session_dir, event->client_uuid);

#if DEBUG
	printf("%s() : saving bank to %s\n", __FUNCTION__, filename);
#endif
	
	amsynth_save_bank(filename);

	char exe_path[4096] = "";
	readlink("/proc/self/exe", exe_path, sizeof(exe_path));

	// construct a command line that the session manager can use to re-launch the synth
	asprintf(&event->command_line,
		"%s -b \"${SESSION_DIR}%s.amsynth.bank\" -P %d -U %s",
		exe_path, event->client_uuid, amsynth_get_preset_number(), event->client_uuid);

#if DEBUG
	printf("%s() : jack_session command_line = %s\n", __FUNCTION__, event->command_line);
#endif
	
	jack_session_reply( (jack_client_t *)arg, event );
	
	switch (event->type)
	{
	case JackSessionSave:
		break;
	case JackSessionSaveAndQuit:
		exit(0);
		break;
	case JackSessionSaveTemplate:
		break;
	}
	
	jack_session_event_free (event);
}

#endif
