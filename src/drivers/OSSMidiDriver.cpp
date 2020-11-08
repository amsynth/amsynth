/*
 *  OSSMidiDriver.cpp
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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "../Configuration.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include "OSSMidiDriver.h"

class OSSMidiDriver : public MidiDriver
{
public:
  	~OSSMidiDriver() override;
	
	int open() override;
	int close() override;
	
	int read(unsigned char *bytes, unsigned maxBytes) override;
	int write_cc(unsigned int channel, unsigned int param, unsigned int value) override;
	
private:
	int _fd = -1;
};


OSSMidiDriver::~OSSMidiDriver()
{
    close();
}

int OSSMidiDriver::open()
{
	Configuration & config = Configuration::get();
	
	if (_fd == -1) 
	{
		const char *dev = config.oss_midi_device.c_str();
		_fd = ::open(dev, O_RDONLY | O_NONBLOCK, 0);
	}
	if (_fd < 0) {
		return -1;
	}
	if (config.current_midi_driver.empty()) {
		config.current_midi_driver = "OSS";
	} else {
		config.current_midi_driver += " + OSS";
	}
	return 0;
}

int OSSMidiDriver::close()
{
	if (_fd > -1)
	{
		::close(_fd);
		_fd = -1;
	}
	return 0;
}

int OSSMidiDriver::read(unsigned char *bytes, unsigned maxBytes)
{
    return (int) ::read(_fd, bytes, maxBytes);
}


int OSSMidiDriver::write_cc(unsigned int /*channel*/, unsigned int /*param*/, unsigned int /*value*/)
{
    return -1;
}

MidiDriver* CreateOSSMidiDriver() { return new OSSMidiDriver; }
