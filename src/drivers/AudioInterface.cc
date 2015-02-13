/*
 *  AudioInterface.cc
 *
 *  Copyright (c) 2001-2015 Nick Dowell
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

#include "../Config.h"
#include "ALSAAudioDriver.h"
#include "ALSAmmapAudioDriver.h"
#include "OSSAudioDriver.h"

#include <cstdlib>
#include <iostream>

using namespace std;


#define SAFE_DELETE(o) if (o) { delete o; o = 0; }


int
AudioInterface::setRealtime()
{
	if (_driver)
		return _driver->setRealtime();
	else
		return -1;
}

bool
try_driver(AudioDriver * ad, Config & cfg)
{
	if (ad->open(cfg) != 0)
		return false;
	
	const size_t numFrames = AudioDriver::kMaxWriteFrames;
	void *buffer = calloc(numFrames, cfg.channels * sizeof(float));
	int write_res = ad->write((float *)buffer, numFrames);
	free(buffer);
	
	return write_res == 0;
}

int
AudioInterface::open( Config & config )
{
	SAFE_DELETE(_driver);
	
	// try ALSA-MMAP
	if (config.audio_driver == "auto" ||
		config.audio_driver == "alsa-mmap")
	{	
		_driver = new ALSAmmapAudioDriver;
		if (try_driver(_driver, config))
		{
			if (config.debug_drivers)
				cout << "<AudioInterface> opened ALSA-MMAP AudioDriver" << endl;
			return 0;
		}
	}
	
	SAFE_DELETE(_driver);
	
	// try ALSA
	if (config.audio_driver == "auto" ||
		config.audio_driver == "alsa")
	{	
		_driver = new ALSAAudioDriver;
		if (try_driver(_driver, config))
		{
			if (config.debug_drivers)
				cout << "<AudioInterface> opened ALSA AudioDriver" << endl;
			return 0;
		}
	}
	
	SAFE_DELETE(_driver);
	
	//try OSS
	if (config.audio_driver == "auto" ||
		config.audio_driver == "oss")
	{	
		_driver = new OSSAudioDriver;
		if (try_driver(_driver, config))
		{
			if (config.debug_drivers)
				cout << "<AudioInterface> opened OSS AudioDriver" << endl;
			return 0;
		}
	}
	
	SAFE_DELETE(_driver);
	
	cerr << "error: could not start audio driver: " << config.audio_driver << "\n";
	config.current_audio_driver = ""; // so the GUI know there is a problem
	return -1;
}

void
AudioInterface::close()
{
	if (_driver) {
		_driver->close();
		delete _driver;
		_driver = NULL;
	}
}

int
AudioInterface::write(float *buffer, int frames)
{
	if (_driver)
		return _driver->write(buffer, frames);
	return -1;
}
