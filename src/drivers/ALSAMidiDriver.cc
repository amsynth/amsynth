/* amSynth
 * (c) 2001 Nick Dowell
 **/

#include "ALSAMidiDriver.h"

#include <unistd.h>

int
ALSAMidiDriver::read(unsigned char *midi_event_buffer)
{
#ifdef _ALSA
	snd_seq_event_t *ev;
	if ( poll(pfd, npfd, 100000) > 0 ){
		snd_seq_event_input( seq_handle, &ev );
		//printf( "%x\n", ev->data );
		_bytes_read = snd_midi_event_decode( seq_midi_parser, 
											midi_event_buffer, 4, ev );
		snd_seq_free_event( ev );
    }
	return _bytes_read;
#else
	cout << "dummy read" << endl;
	return -1;
#endif
}

int ALSAMidiDriver::close()
{
	return 0;
}

int ALSAMidiDriver::open(string device)
{
#ifdef _ALSA
	if (snd_seq_open(&seq_handle, "hw", SND_SEQ_OPEN_DUPLEX, 0) < 0) {
		cerr << "Error opening ALSA sequencer.\n";
		exit(1);
	}
	
	snd_seq_set_client_name(seq_handle, "amSynth");
  
	if ((portid = snd_seq_create_simple_port(seq_handle, "amSynth Midi In",
            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE,
            SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
		cerr << "Error creating sequencer port.\n";
		exit(1);
	}
	
	npfd = snd_seq_poll_descriptors_count(seq_handle, POLLIN);
	pfd = (struct pollfd *)alloca(npfd * sizeof(struct pollfd));
	snd_seq_poll_descriptors(seq_handle, pfd, npfd, POLLIN);
	
	cerr << "alsamidi open() done\n";
	return 0;
#else
	cout << "ALSAMidiDriver::open() dummy\n";
	return -1;
#endif
}

ALSAMidiDriver::ALSAMidiDriver()
{
	if( snd_midi_event_new( 4, &seq_midi_parser ) )
		cout << "Error creating midi event parser\n";
}

ALSAMidiDriver::~ALSAMidiDriver()
{
	close();
}