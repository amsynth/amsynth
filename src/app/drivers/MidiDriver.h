/*
 *  MidiDriver.h
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

#ifndef _MIDI_DRIVER_H
#define _MIDI_DRIVER_H

class MidiDriver
{
public:
	virtual ~MidiDriver () {}
	
    // read() returns the number of bytes succesfully read. numbers < 0 
    // generally indicate failure...
    virtual int read(unsigned char *bytes, unsigned maxBytes) = 0;
    virtual int write_cc(unsigned int channel, unsigned int param, unsigned int value) = 0;
    virtual int open() = 0;
    virtual int close() = 0;
};

#endif

