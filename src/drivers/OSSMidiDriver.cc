/* amSynth
 * (c) 2001,2002 Nick Dowell
 **/
#include "OSSMidiDriver.h"

int
 OSSMidiDriver::read(unsigned char *midi_event_buffer)
{
    int bar =::read(_oss_midi_fd, midi_event_buffer, MIDI_BUF_SIZE);
    return bar;
}

int OSSMidiDriver::close()
{
    if (opened) {
	int foo =::close(_oss_midi_fd);
	return foo;
    } else
	return 0;
}

int OSSMidiDriver::open( Config & config )
{
    _oss_midi_fd =::open(config.oss_audio_device.c_str(), O_RDONLY, 0);
    if (_oss_midi_fd == -1)
	return (-1);
    else {
	opened = 1;
#ifdef _DEBUG
	cout << "<OSSMidiDriver> opened device " << config.oss_audio_device << endl;
#endif
	return 0;
    }
}

OSSMidiDriver::OSSMidiDriver()
{
    opened = 0;
}

OSSMidiDriver::~OSSMidiDriver()
{
    close();
}
