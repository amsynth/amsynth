/* Synth--
 * (c) 2001-2006 Nick Dowell
 **/

#ifndef _OSS_MIDI_DRIVER_H
#define _OSS_MIDI_DRIVER_H

#include "MidiDriver.h"

class OSSMidiDriver : public MidiDriver
{
public:
	OSSMidiDriver();
  	virtual ~OSSMidiDriver();

	int open( Config & config );
	int close();

	int read(unsigned char *bytes, unsigned maxBytes);
	int write_cc(unsigned int channel, unsigned int param, unsigned int value);
  
private:
	int _fd;
};

#endif // _OSS_MIDI_DRIVER_H
