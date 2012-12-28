/*
 *  AudioOutput.cc
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

#include "AudioOutput.h"

AudioOutput::AudioOutput()
:	buffer (NULL)
{
	running = 0;
	recording = 0;
	wavoutfile = "/tmp/amSynth.wav";
}

AudioOutput::~AudioOutput()
{
	out.close();
	delete[] buffer;
}

int
AudioOutput::init	(Config & config)
{
	this->config = &config;
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
	if (out.open (*config) == -1) return false;
	out.setRealtime();
	if (0 != Thread::Run ())
	{
		out.close ();
		return false;
	}
	return true;
}

void
AudioOutput::Stop ()
{
	Thread::Kill ();
	Thread::Join ();
	out.close ();
}

void 
AudioOutput::ThreadAction	()
{
	int bufsize = config->buffer_size;
	while (!ShouldStop ())
	{
		if (mAudioCallback != NULL)
			(*mAudioCallback)(buffer+bufsize*2, buffer+bufsize*3, bufsize, 1);

		for (int i=0; i<bufsize; i++) {
			buffer[2*i]   = buffer[bufsize*2+i];
			buffer[2*i+1] = buffer[bufsize*3+i];
		}

#ifdef with_sndfile
		if (recording) sf_writef_float (sndfile, buffer, bufsize);
#endif
		if (out.write (buffer, bufsize*channels) == -1) Stop ();
	}
}
