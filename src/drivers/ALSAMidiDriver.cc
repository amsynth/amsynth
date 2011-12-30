/* amSynth
 * (c) 2001-2006 Nick Dowell
 **/

#include "ALSAMidiDriver.h"

#ifdef WITH_ALSA

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
	ALSAMidiDriver		( );
	virtual ~ALSAMidiDriver		( );
	int 	read			(unsigned char *bytes, unsigned maxBytes);
	int		write_cc		(unsigned int channel, unsigned int param, unsigned int value);
	int 	open			( Config & config );
	int 	close			( );
	int 	get_alsa_client_id	( )	{ return client_id; };
	bool	input_pending	( );
private:
	snd_seq_t		*seq_handle;
	snd_midi_event_t	*seq_midi_parser;
	int 			portid;
	int				portid_out;
	int			client_id;
	struct pollfd pollfd_in;
};

bool
ALSAMidiDriver::input_pending()
{
	return (0 < snd_seq_event_input_pending( seq_handle, 1 ))
		|| (0 < poll( &pollfd_in, 1, 100 ));
}

int
ALSAMidiDriver::read(unsigned char *bytes, unsigned maxBytes)
{
	int bytes_read = 0;
	if (input_pending() && seq_handle) {
		snd_seq_event_t *ev = NULL;
		snd_seq_event_input( seq_handle, &ev );
		bytes_read = snd_midi_event_decode( seq_midi_parser, bytes, maxBytes, ev );
		snd_seq_free_event( ev );
	}
	return bytes_read;
}

int
ALSAMidiDriver::write_cc(unsigned int channel, unsigned int param, unsigned int value)
{
      int ret=0;
      client_id = 0;
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
      snd_seq_free_event( &ev );

      return ret;
}



int ALSAMidiDriver::close()
{
	if (seq_handle) snd_seq_close (seq_handle);
	seq_handle = NULL;
	return 0;
}

int ALSAMidiDriver::open( Config & config )
{
	if (seq_handle) return 0;
	
	if (snd_seq_open(&seq_handle, "default", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
		cerr << "Error opening ALSA sequencer.\n";
		return -1;
	}
	
	snd_seq_set_client_name(seq_handle, config.alsa_seq_client_name.c_str());
	
	string port_name = config.alsa_seq_client_name;
	port_name += " MIDI IN";

	if ((portid = snd_seq_create_simple_port(seq_handle, port_name.c_str(),
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
		cerr << "Error creating sequencer port.\n";
		return -1;
	}

	client_id = snd_seq_client_id( seq_handle );
	
//	if (config.debug_drivers)
//		cerr << "opened alsa sequencer client. id=" << client_id << endl;

	port_name = config.alsa_seq_client_name;
	port_name += " MIDI OUT";
	
	if ((portid_out = snd_seq_create_simple_port(seq_handle, port_name.c_str(),
		SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ,
		SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
		cerr << "Error creating sequencer port.\n";
		return -1;
	}
	
	snd_seq_poll_descriptors( seq_handle, &pollfd_in, 1, POLLIN );

	return 0;
}

ALSAMidiDriver::ALSAMidiDriver()
{
	seq_handle = NULL;
	memset( &pollfd_in, 0, sizeof(pollfd_in) );
	if( snd_midi_event_new( 32, &seq_midi_parser ) )
		cout << "Error creating midi event parser\n";
}

ALSAMidiDriver::~ALSAMidiDriver()
{
	close();
}

MidiDriver* CreateAlsaMidiDriver() { return new ALSAMidiDriver; }
#else
MidiDriver* CreateAlsaMidiDriver() { return NULL; }
#endif
