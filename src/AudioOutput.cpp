/*
 *  AudioOutput.cpp
 *
 *  Copyright (c) 2001-2020 Nick Dowell
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

#include "AudioOutput.h"

#include "Configuration.h"
#include "drivers/AudioDriver.h"
#include "drivers/ALSAAudioDriver.h"
#include "drivers/ALSAmmapAudioDriver.h"
#include "drivers/OSSAudioDriver.h"

#include <cstdlib>


static AudioDriver * open_driver();


AudioOutput::~AudioOutput()
{
	Stop();
	delete[] buffer;
}

int
AudioOutput::init()
{
	Configuration & config = Configuration::get();
	channels = config.channels;

	if (buffer) delete[] buffer;
	buffer = new float [config.buffer_size*4];
	
	return 0;
}

bool
AudioOutput::Start ()
{
	if (!(driver = open_driver())) {
		return false;
	}
	shouldStop = false;
	thread = std::thread(&AudioOutput::ThreadAction, this);
	return true;
}

void
AudioOutput::Stop ()
{
	shouldStop = true;
	if (thread.joinable()) {
		thread.join();
	}

	if (driver) {
		driver->close();
		delete driver;
		driver = nullptr;
	}
}

void 
AudioOutput::ThreadAction	()
{
	Configuration & config = Configuration::get();
	int bufsize = config.buffer_size;
	while (!shouldStop) {
		std::vector<amsynth_midi_event_t> midi_in;
		std::vector<amsynth_midi_cc_t> midi_out;
		amsynth_audio_callback(buffer+bufsize*2, buffer+bufsize*3, bufsize, 1, midi_in, midi_out);

		for (int i=0; i<bufsize; i++) {
			buffer[2*i]   = buffer[bufsize*2+i];
			buffer[2*i+1] = buffer[bufsize*3+i];
		}

		if (driver->write(buffer, bufsize * channels) < 0) {
			break;
		}
	}
}

static AudioDriver * open_driver(AudioDriver *driver)
{
	Configuration & config = Configuration::get();
	if (!driver) {
		return nullptr;
	}
	if (driver->open() != 0) {
		delete driver;
		return nullptr;
	}
	void *buffer = calloc(AudioDriver::kMaxWriteFrames, config.channels * sizeof(float));
	int written = driver->write((float *)buffer, AudioDriver::kMaxWriteFrames);
	free(buffer);
	if (written != 0) {
		delete driver;
		return nullptr;
	}
	return driver;
}

static AudioDriver * open_driver()
{
	AudioDriver *driver = nullptr;
	Configuration & config = Configuration::get();

	if (config.audio_driver == "alsa-mmap" || config.audio_driver == "auto") {
        if ((driver = open_driver(CreateALSAmmapAudioDriver()))) {
			return driver;
		}
	}

	if (config.audio_driver == "alsa" || config.audio_driver == "auto") {
        if ((driver = open_driver(CreateALSAAudioDriver()))) {
			return driver;
		}
	}

	if (config.audio_driver == "oss" || config.audio_driver == "auto") {
        if ((driver = open_driver(CreateOSSAudioDriver()))) {
			return driver;
		}
	}

	return nullptr;
}
