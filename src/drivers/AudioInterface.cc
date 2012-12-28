/*
 *  AudioInterface.cc
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

#include "AudioInterface.h"

#include <cstdlib>

#define SAFE_DELETE(o) if (o) { delete o; o = 0; }

int
AudioInterface::setRealtime()
{
	if (driver)
		return driver->setRealtime();
	else
		return -1;
}

int
AudioInterface::setRate(int rate)
{
	if (driver)
		return driver->setRate(rate);
	else
		return -1;
}

int
AudioInterface::setChannels(int channels)
{
	if (driver)
		return driver->setChannels(channels);
	else
		return -1;
}

bool
try_driver(AudioDriver * ad, Config & cfg)
{
	if (ad->open(cfg) != 0)
		return false;
	
	const size_t numFrames = 1024;
	void *buffer = calloc(numFrames, 8);
	int write_res = ad->write((float *)buffer, numFrames);
	free(buffer);
	
	return write_res == 0;
}

int
AudioInterface::open( Config & config )
{
	SAFE_DELETE(driver);
	
	// try ALSA-MMAP
	if (config.audio_driver == "auto" ||
		config.audio_driver == "alsa-mmap")
	{	
		driver = new ALSAmmapAudioDriver;
		if (try_driver(driver, config))
		{
			if (config.debug_drivers)
				cout << "<AudioInterface> opened ALSA-MMAP AudioDriver" << endl;
			return 0;
		}
	}
	
	SAFE_DELETE(driver);
	
	// try ALSA
	if (config.audio_driver == "auto" ||
		config.audio_driver == "alsa")
	{	
		driver = new ALSAAudioDriver;
		if (try_driver(driver, config))
		{
			if (config.debug_drivers)
				cout << "<AudioInterface> opened ALSA AudioDriver" << endl;
			return 0;
		}
	}
	
	SAFE_DELETE(driver);
	
	//try OSS
	if (config.audio_driver == "auto" ||
		config.audio_driver == "oss")
	{	
		driver = new OSSAudioDriver;
		if (try_driver(driver, config))
		{
			if (config.debug_drivers)
				cout << "<AudioInterface> opened OSS AudioDriver" << endl;
			return 0;
		}
	}
	
	SAFE_DELETE(driver);
	
	cerr << "error: could not start audio driver: " << config.audio_driver << "\n";
	config.current_audio_driver = ""; // so the GUI know there is a problem
	return -1;
}

void
AudioInterface::close()
{
	if (driver) driver->close();
}

AudioInterface::AudioInterface()
{
	_no_of_drivers = 2;
	_drivers = new int[_no_of_drivers];
	_drivers[0] = AUDIO_DRIVER_ALSA;
	_drivers[1] = AUDIO_DRIVER_OSS;
	driver = 0;
}

AudioInterface::~AudioInterface()
{
	SAFE_DELETE(driver);
}
