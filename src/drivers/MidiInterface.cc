/* amSynth
 * (c) 2001 Nick Dowell
 **/

#include "MidiInterface.h"

int
MidiInterface::read(unsigned char *buffer)
{
	return midi->read(buffer);
}

void MidiInterface::close()
{
	midi->close();
#ifdef _DEBUG
	cout << "<MidiInterface::close()> closed Midi Device" << endl;
#endif 
	//  delete midi; // this is (was) causing a Segfault....
}

int MidiInterface::open( Config & config, string name )
{
	if (config.midi_driver == "auto")
	{
		//try ALSA
		midi = new ALSAMidiDriver;
#ifdef _DEBUG
		cout << "<MidiInterface> Trying to open ALSA midi device...\n";
#endif
		if ((midi->open("",name)) == 0)
		{
			config.midi_driver = "ALSA";
			config.alsa_seq_client_id = midi->get_alsa_client_id();
#ifdef _DEBUG
			cout << "<MidiInterface> opened ALSA midi device\n";
#endif
			return 0;
		}
#ifdef _DEBUG
		cout << "<MidiInterface> failed to open ALSA midi device, " <<
		"falling back to OSS.." << endl;
#endif
		delete midi;

		//try OSS
		midi = new OSSMidiDriver;
		if ((midi->open(config.oss_midi_device,name)) == 0) 
		{
			config.midi_driver = "OSS";
#ifdef _DEBUG
			cout << "<MidiInterface> opened OSS midi device\n";
#endif
			return 0;
		}

		cout << "<MidiInterface> failed to open OSS midi device.\n";
		cout << "<MidiInterface> couldn't open any MIDI drivers\n";
		return -1;
	}
	else if (config.midi_driver == "oss" || config.midi_driver == "OSS")
	{
		midi = new OSSMidiDriver;
		if ((midi->open(config.oss_midi_device,name)) == 0)
		{
			config.midi_driver = "OSS";
#ifdef _DEBUG
			cout << "<MidiInterface> opened OSS midi device\n";
#endif
			return 0;
		}

		cout << "<MidiInterface> failed to open OSS midi device.\n";
		cout << "<MidiInterface> couldn't open any MIDI drivers :-(\n";
		return -1;
	} 
	else if (config.midi_driver == "alsa" || config.midi_driver == "ALSA")
	{
		midi = new ALSAMidiDriver;
#ifdef _DEBUG
		cout << "<MidiInterface> Trying to open ALSA midi device...\n";
#endif
		if ((midi->open("",name)) == 0)
		{
			config.midi_driver = "ALSA";
			config.alsa_seq_client_id = midi->get_alsa_client_id();
#ifdef _DEBUG
			cout << "<MidiInterface> opened ALSA midi device!\n";
#endif
			return 0;
		}
		return -1;
	}
	return -1;
}

MidiInterface::MidiInterface()
{
}

MidiInterface::~MidiInterface()
{
    close();
}
