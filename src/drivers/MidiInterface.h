/*
 *  MidiInterface.h
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

#ifndef _MidiInterface_h
#define _MidiInterface_h

#include "../Config.h"

class MidiStreamReceiver
{
public:
	MidiStreamReceiver() : _midiIface(0) {}
	virtual ~MidiStreamReceiver() {}
	virtual void HandleMidiData(unsigned char* bytes, unsigned numBytes) = 0;
	
	// just a kludge for the time being so that we can still do MIDI out:
	virtual void SetMidiInterface(class MidiInterface* in) { _midiIface = in; }
protected:
	class MidiInterface *_midiIface;
};


class MidiInterface
{
public:
    MidiInterface();
    virtual ~MidiInterface();
	
	virtual int open(Config&);
	virtual void close();
	
	virtual void SetMidiStreamReceiver(MidiStreamReceiver* in);

	virtual void poll();
    virtual int write_cc(unsigned int channel, unsigned int param, unsigned int value);
	
protected:

	MidiStreamReceiver* _handler;
	
private:
	class MidiDriver * midi;
	unsigned char *_buffer;
};

#endif
