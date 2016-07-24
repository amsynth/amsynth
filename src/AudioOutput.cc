/*
 *  AudioOutput.cc
 *
 *  Copyright (c) 2001-2016 Nick Dowell
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

#include <stdlib.h>


static AudioDriver * open_driver();


AudioOutput::AudioOutput()
:	buffer (NULL)
,	driver(NULL)
{
	running = 0;
	recording = 0;
	wavoutfile = "/tmp/amSynth.wav";
}

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
	
#ifdef with_sndfile
	sf_info.samplerate = config.sample_rate;
	sf_info.channels = config.channels;
	
#ifdef SNDFILE_1
	// libsndfile version 1.x:
	sf_info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
#else
	// libsndfile versions < 1.0:
	sf_info.format = SF_FORMAT_WAV | SF_FORMAT_PCM;
	sf_info.pcmbitwidth = 16;
#endif
#endif

	if (buffer) delete[] buffer;
	buffer = new float [config.buffer_size*4];
	
	return 0;
}

void
AudioOutput::startRecording()
{
#ifdef with_sndfile
#ifdef SNDFILE_1
	// libsndfile version 1.x:
	sndfile = sf_open( wavoutfile.c_str(), SFM_WRITE, &sf_info );

	sf_command( sndfile, SFC_SET_NORM_FLOAT, NULL, SF_TRUE );
#else
	// libsndfile versions < 1.0:
	sndfile = sf_open_write( wavoutfile.c_str(), &sf_info );
	sf_command( sndfile, "norm float", (void*)"on", 0 );
#endif
	recording = 1;
#endif
}

void
AudioOutput::stopRecording()
{
	recording = 0;
#ifdef with_sndfile
	sf_close( sndfile );
#endif
}

bool
AudioOutput::Start ()
{
	if (!(driver = open_driver())) {
		return false;
	}
	driver->setRealtime();
	if (Thread::Run() != 0) {
		driver->close();
		driver = NULL;
		return false;
	}
	return true;
}

void
AudioOutput::Stop ()
{
	Thread::Stop();
	Thread::Join();

	if (driver) {
		driver->close();
		driver = NULL;
	}
}

void 
AudioOutput::ThreadAction	()
{
	Configuration & config = Configuration::get();
	int bufsize = config.buffer_size;
	while (!ShouldStop ()) {
		std::vector<amsynth_midi_event_t> midi_in;
		std::vector<amsynth_midi_cc_t> midi_out;
		amsynth_audio_callback(buffer+bufsize*2, buffer+bufsize*3, bufsize, 1, midi_in, midi_out);

		for (int i=0; i<bufsize; i++) {
			buffer[2*i]   = buffer[bufsize*2+i];
			buffer[2*i+1] = buffer[bufsize*3+i];
		}

#ifdef with_sndfile
		if (recording) sf_writef_float (sndfile, buffer, bufsize);
#endif

		if (driver->write(buffer, bufsize * channels) < 0) {
			Stop();
		}
	}
}

static AudioDriver * open_driver(AudioDriver *driver)
{
	Configuration & config = Configuration::get();
	if (!driver) {
		return NULL;
	}
	if (driver->open() != 0) {
		delete driver;
		return NULL;
	}
	void *buffer = calloc(AudioDriver::kMaxWriteFrames, config.channels * sizeof(float));
	int written = driver->write((float *)buffer, AudioDriver::kMaxWriteFrames);
	free(buffer);
	if (written != 0) {
		delete driver;
		return NULL;
	}
	return driver;
}

static AudioDriver * open_driver()
{
	AudioDriver *driver = NULL;
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

	return NULL;
}
