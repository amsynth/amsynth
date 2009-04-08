/* amSynth
 * (c) 2001-2006 Nick Dowell
 **/

#include "MidiInterface.h"

#include "ALSAMidiDriver.h"
#include "OSSMidiDriver.h"

#include <iostream>
#include <cstring>

#define MIDI_BUF_SIZE 64
void sched_realtime (); // defined in main.cc
using namespace std;

int MidiInterface::write_cc(unsigned int channel, unsigned int param, unsigned int val)
{
	return (midi) ? midi->write_cc(channel, param, val) : -7;
}

int MidiInterface::open( Config & config )
{
	if (midi) return 0;
	
	if (config.midi_driver == "auto")
	{
		//try ALSA
		if (config.debug_drivers) cout << "<MidiInterface> Trying to open ALSA midi device...\n";
		if (NULL != (midi = CreateAlsaMidiDriver()))
		{
			if ((midi->open(config)) == 0)
			{
				config.current_midi_driver = "ALSA";
				config.alsa_seq_client_id = midi->get_alsa_client_id();
				if (config.debug_drivers) cout << "<MidiInterface> opened ALSA midi device\n";
				return 0;
			}
			if (config.debug_drivers) cout << "<MidiInterface> failed to open ALSA midi device, falling back to OSS.." << endl;
			delete midi; midi = NULL;
		}

		//try OSS
		if (NULL != (midi = CreateOSSMidiDriver()))
		{
			if ((midi->open(config)) == 0) 
			{
				config.current_midi_driver = "OSS";
				if (config.debug_drivers) cout << "<MidiInterface> opened OSS midi device\n";
				return 0;
			}
			delete midi; midi = NULL;
		}

		cout << "<MidiInterface> failed to open OSS midi device.\n";
		cout << "<MidiInterface> couldn't open any MIDI drivers\n";
		return -1;
	}
	else if (config.midi_driver == "oss" || config.midi_driver == "OSS")
	{
		if (NULL != (midi = CreateOSSMidiDriver()))
		{
			if ((midi->open(config)) == 0) 
			{
				config.current_midi_driver = "OSS";
				if (config.debug_drivers) cout << "<MidiInterface> opened OSS midi device\n";
				return 0;
			}
			delete midi; midi = NULL;
		}
		cout << "<MidiInterface> failed to open OSS midi device.\n";
		cout << "<MidiInterface> couldn't open any MIDI drivers :-(\n";
		return -1;
	} 
	else if (config.midi_driver == "alsa" || config.midi_driver == "ALSA")
	{
		if (NULL != (midi = CreateAlsaMidiDriver()))
		{
			if ((midi->open(config)) == 0)
			{
				config.current_midi_driver = "ALSA";
				config.alsa_seq_client_id = midi->get_alsa_client_id();
				if (config.debug_drivers) cout << "<MidiInterface> opened ALSA midi device\n";
				return 0;
			}
			if (config.debug_drivers) cout << "<MidiInterface> failed to open ALSA midi device, falling back to OSS.." << endl;
			delete midi; midi = NULL;
		}
	}
	return -1;
}


void MidiInterface::close()
{
	if (midi) {
		midi->close();
		delete midi;
	}
}

void
MidiInterface::ThreadAction ()
{
	sched_realtime ();
    while (!ShouldStop () && midi)
	{
		bzero(_buffer, MIDI_BUF_SIZE);
		int bytes_read = midi->read(_buffer, MIDI_BUF_SIZE);
		if (bytes_read == -1)
		{
			cout << "error reading from midi device" << endl;
			break;
		}
		if (bytes_read > 0 && _handler) _handler->HandleMidiData(_buffer, bytes_read);
	}
}

// need to kill the thread, otherwise it waits indefinitely for the next incoming byte
void MidiInterface::Stop ()
{ 
	Thread::Kill (); 
	Thread::Join ();
}

void MidiInterface::SetMidiStreamReceiver(MidiStreamReceiver* in)
{
	if (_handler) _handler->SetMidiInterface(NULL);
	_handler = in;
	if (_handler) _handler->SetMidiInterface(this);
}
	
MidiInterface::MidiInterface()
:	_handler(NULL)
,	midi(NULL)
{
	_buffer = new unsigned char[MIDI_BUF_SIZE];
}

MidiInterface::~MidiInterface()
{
	delete[] _buffer;
    close();
}

