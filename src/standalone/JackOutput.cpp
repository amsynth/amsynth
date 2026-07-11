/*
 *  JackOutput.cpp
 *
 *  Copyright (c) 2001-2023 Nick Dowell
 *
 *  This file is part of amsynth.
 *
 *  amsynth is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  amsynth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with amsynth.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "JackOutput.h"

#include "core/Configuration.h"
#include "core/midi.h"

#if HAVE_JACK_MIDIPORT_H
#include <jack/midiport.h>
#endif

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <vector>


#define UNUSED_PARAM( x ) (void)x


int
JackOutput::init()
{
	Configuration & config = Configuration::get();
	
#ifdef WITH_JACK
	if (client) // already initialised
		return 0;

	l_port = r_port = m_port = nullptr;

	jack_status_t status = (jack_status_t)0;
	client = jack_client_open(config.jack_client_name_preference.c_str(), JackNoStartServer, &status);
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
	m_port_out = jack_port_register(client, "midi_out", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0);
#endif
	
	config.current_audio_driver = "JACK";
	if (m_port) {
		config.current_midi_driver = "JACK";
	}
	config.sample_rate = jack_get_sample_rate(client);
	config.buffer_size = jack_get_buffer_size(client);
	config.jack_client_name = std::string(jack_get_client_name(client));

	return 0;
#endif
	UNUSED_PARAM(config);
	return -1;
}

#ifdef WITH_JACK

static inline amsynth_midi_event_t amsynth_midi_event_make(jack_midi_event_t je)
{
	amsynth_midi_event_t me;
	me.offset_frames = je.time;
	me.length = je.size;
	me.buffer = je.buffer;
	return me;
}

int
JackOutput::process (jack_nframes_t nframes, void *arg)
{
	JackOutput *self = (JackOutput *)arg;
	std::vector<amsynth_midi_event_t> midi_events;
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
			midi_events.push_back(amsynth_midi_event_make(midi_event));
		}
	}
#endif
	std::vector<amsynth_midi_cc_t> midi_out;
	amsynth_audio_callback(lout, rout, nframes, 1, midi_events, midi_out);
#if HAVE_JACK_MIDIPORT_H
	if (self->m_port_out) {
		void *port_buffer = jack_port_get_buffer(self->m_port_out, nframes);
		jack_midi_clear_buffer(port_buffer);
		std::vector<amsynth_midi_cc_t>::const_iterator out_it;
		for (out_it = midi_out.begin(); out_it != midi_out.end(); ++out_it) {
			jack_midi_data_t data[] = {
				(unsigned char) (MIDI_STATUS_CONTROLLER | (out_it->channel & 0x0f)),
				out_it->cc, out_it->value };
			jack_midi_event_write(port_buffer, 0, data, 3);
		}
	}
#endif
	return 0;
}
#endif

bool 
JackOutput::Start	()
{
#ifdef WITH_JACK
	if (!client) return false;
	if (jack_activate(client)) 
	{
		std::cerr << "cannot activate JACK client\n";
		return false;
	}
	if (Configuration::get().jack_autoconnect) {
		const char **port_names = jack_get_ports(client, nullptr, JACK_DEFAULT_AUDIO_TYPE, JackPortIsPhysical | JackPortIsInput);
		if (port_names) {
			jack_connect(client, jack_port_name(l_port), port_names[0]);
			jack_connect(client, jack_port_name(r_port), port_names[1]);
			jack_free(port_names);
		}

		port_names = jack_get_ports(client, nullptr, JACK_DEFAULT_MIDI_TYPE, JackPortIsPhysical | JackPortIsOutput);
		if (port_names) {
			for (int i = 0; port_names[i]; i++) { const char *port = port_names[i];
				jack_connect(client, port, jack_port_name(m_port));
			}
			jack_free(port_names);
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
	client = nullptr;
#endif
}
