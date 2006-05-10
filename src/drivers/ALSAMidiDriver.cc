/* amSynth
 * (c) 2001-2006 Nick Dowell
 **/

#include "ALSAMidiDriver.h"
#include <unistd.h>

using namespace std;

int
ALSAMidiDriver::read(unsigned char *bytes, unsigned maxBytes)
{
	client_id = 0;
#ifdef with_alsa
	snd_seq_event_t *ev;
	
	snd_seq_event_input( seq_handle, &ev );
	_bytes_read = snd_midi_event_decode(seq_midi_parser, bytes, maxBytes, ev);
	snd_seq_free_event( ev );

	return _bytes_read;
#else
	return -1;
#endif
}

int
ALSAMidiDriver::write_cc(unsigned int channel, unsigned int param, unsigned int value)
{
      int ret=0;
      client_id = 0;
#ifdef with_alsa
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
#else
      return -1;
#endif
}



int ALSAMidiDriver::close()
{
#ifdef with_alsa
	if (seq_handle) snd_seq_close (seq_handle);
	seq_handle = NULL;
#endif
	return 0;
}

int ALSAMidiDriver::open( Config & config )
{
#ifdef with_alsa
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

	return 0;
#else
	return -1;
#endif
}

ALSAMidiDriver::ALSAMidiDriver()
{
#ifdef with_alsa
	seq_handle = NULL;
	if( snd_midi_event_new( 32, &seq_midi_parser ) )
		cout << "Error creating midi event parser\n";
#endif
}

ALSAMidiDriver::~ALSAMidiDriver()
{
	close();
}
