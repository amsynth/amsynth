/* amSynth
 * (c) 2001-2003 Nick Dowell
 **/

#include "ALSAMidiDriver.h"

#include <unistd.h>

int
ALSAMidiDriver::read(unsigned char *midi_event_buffer)
{
	client_id = 0;
#ifdef with_alsa
	snd_seq_event_t *ev;
	
	snd_seq_event_input( seq_handle, &ev );
	_bytes_read = snd_midi_event_decode
		( seq_midi_parser, midi_event_buffer, 32, ev );
	snd_seq_free_event( ev );

	return _bytes_read;
#else
	return -1;
#endif
}

int ALSAMidiDriver::close()
{
#ifdef with_alsa
	snd_seq_close (seq_handle);
#endif
	return 0;
}

int ALSAMidiDriver::open( Config & config )
{
#ifdef with_alsa
	if (snd_seq_open(&seq_handle, "hw", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
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

	return 0;
#else
	return -1;
#endif
}

ALSAMidiDriver::ALSAMidiDriver()
{
#ifdef with_alsa
	if( snd_midi_event_new( 32, &seq_midi_parser ) )
		cout << "Error creating midi event parser\n";
#endif
}

ALSAMidiDriver::~ALSAMidiDriver()
{
	close();
}
