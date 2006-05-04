/* amSynth
 * (c) 2001-2006 Nick Dowell
 **/

#ifndef _MIDI_DRIVER_H
#define _MIDI_DRIVER_H

#include "../Config.h"

class MidiDriver
{
public:
	virtual ~MidiDriver () {}
	
    // read() returns the number of bytes succesfully read. numbers < 0 
    // generally indicate failure...
    virtual int read(unsigned char *bytes, unsigned maxBytes) = 0;
    virtual int write_cc(unsigned int channel, unsigned int param, unsigned int value) = 0;
    virtual int open( Config & config ) = 0;
    virtual int close() = 0;
    virtual int get_alsa_client_id()	{ return 0; };
};

#endif

