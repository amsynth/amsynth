/*
 *  ALSAMidiDriver.cpp
 *
 *  Copyright (c) 2001-2012 Nick Dowell
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ALSAMidiDriver.h"

#ifdef WITH_ALSA

#include "../Configuration.h"

#define ALSA_PCM_OLD_HW_PARAMS_API
#define ALSA_PCM_OLD_SW_PARAMS_API
#include <alsa/asoundlib.h>
#include <unistd.h>
#include <iostream>
#include <poll.h>

using namespace std;


class ALSAMidiDriver : public MidiDriver
{
public:
	ALSAMidiDriver(const char *client_name);
	~ALSAMidiDriver( ) override;
    int read(unsigned char *buffer, unsigned maxBytes) override;
    int write_cc(unsigned int channel, unsigned int param, unsigned int value) override;
    int open() override;
    int close() override;
private:
	const char		*client_name;
	snd_seq_t		*seq_handle;
	snd_midi_event_t	*seq_midi_parser;
	int 			portid;
	int				portid_out;
	struct pollfd pollfd_in;
};

int
ALSAMidiDriver::read(unsigned char *buffer, unsigned maxBytes)
{
	if (seq_handle == nullptr) {
		return 0;
	}
	unsigned char *ptr = buffer;
	while (1) {
		snd_seq_event_t *ev = nullptr;
		int res = snd_seq_event_input(seq_handle, &ev);
		if (res < 0)
			break;
		ptr += snd_midi_event_decode(seq_midi_parser, ptr, maxBytes - (ptr - buffer), ev);
		if (res < 1)
			break;
	}
	return (int)(ptr - buffer);
}

int
ALSAMidiDriver::write_cc(unsigned int channel, unsigned int param, unsigned int value)
{
	if (seq_handle == nullptr) {
		return 0;
	}
      int ret=0;
      snd_seq_event_t ev;


        snd_seq_ev_clear(&ev);
        snd_seq_ev_set_subs(&ev);
        snd_seq_ev_set_direct(&ev);
        snd_seq_ev_set_source(&ev, portid_out);
        ev.type = SND_SEQ_EVENT_CONTROLLER;
        ev.data.control.channel = channel;
        ev.data.control.param = param;
        ev.data.control.value = value;
        ret=snd_seq_event_output_direct(seq_handle, &ev);
#if _DEBUG
      cout << "param = " << param << " value = " << value << " ret = " << ret << endl;
#endif
      if (ret < 0 ) cout << snd_strerror(ret) << endl;        
      return ret;
}



int ALSAMidiDriver::close()
{
	if (seq_handle) snd_seq_close (seq_handle);
	seq_handle = nullptr;
	return 0;
}

int ALSAMidiDriver::open()
{
	Configuration & config = Configuration::get();

	if (seq_handle) return 0;
	
	if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_DUPLEX, SND_SEQ_NONBLOCK) != 0) {
		cerr << "Error opening ALSA sequencer.\n";
		return -1;
	}

	if (seq_handle == nullptr) {
		cerr << "error: snd_seq_open() claimed to succeed but seq_handle is NULL.\n";
		return -1;
	}
	
	if (client_name) {
		snd_seq_set_client_name(seq_handle, client_name);
	}
	
	if ((portid = snd_seq_create_simple_port(seq_handle, "MIDI IN",
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
		cerr << "Error creating sequencer port.\n";
		return -1;
	}

	if ((portid_out = snd_seq_create_simple_port(seq_handle, "MIDI OUT",
		SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
		SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
		cerr << "Error creating sequencer port.\n";
		return -1;
	}
	
	snd_seq_poll_descriptors( seq_handle, &pollfd_in, 1, POLLIN );

	if (config.current_midi_driver.empty()) {
		config.current_midi_driver = "ALSA";
	} else {
		config.current_midi_driver += " + ALSA";
	}
	config.alsa_seq_client_id = snd_seq_client_id(seq_handle);

	return 0;
}

ALSAMidiDriver::ALSAMidiDriver(const char *client_name)
:	client_name(client_name)
{
	seq_handle = nullptr;
	memset( &pollfd_in, 0, sizeof(pollfd_in) );
	if( snd_midi_event_new( 32, &seq_midi_parser ) )
		cout << "Error creating MIDI event parser\n";
}

ALSAMidiDriver::~ALSAMidiDriver()
{
	close();
}

MidiDriver* CreateAlsaMidiDriver(const char *client_name) { return new ALSAMidiDriver(client_name); }
#else
MidiDriver* CreateAlsaMidiDriver(const char *client_name) { return nullptr; }
#endif
