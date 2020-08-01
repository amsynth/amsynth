/*
 *  OSSAudioDriver.cpp
 *
 *  Copyright (c) 2001-2017 Nick Dowell
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

#include "OSSAudioDriver.h"

#ifdef WITH_OSS

#include "../Configuration.h"
#include "AudioDriver.h"

#include <cstddef>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <unistd.h>


class OSSAudioDriver : public AudioDriver
{
  public:
    int open() override;
    void close() override;
    int write(float *buffer, int frames) override;

  private:
    int _fd = -1;
    unsigned char *_outputBuffer = nullptr;
    unsigned int _outputBufferFrames = 0;
};

#define ON_ERROR do { \
    if (_fd != -1) { \
        ::close(_fd); \
        _fd = -1; \
    } \
    return -1; \
} while (0)

int
OSSAudioDriver::open()
{
    Configuration &config = Configuration::get();
    const char *name = config.oss_audio_device.c_str();

    if ((_fd = ::open(name, O_WRONLY, 0)) == -1) {
        perror("Could not open OSS audio device");
        ON_ERROR;
    }

    // Buffer size

    int fragment = 0x0004000A; // 4 fragments of 256 samples (1024 bytes)

    if (ioctl(_fd, SNDCTL_DSP_SETFRAGMENT, &fragment) == -1) {
        perror("SNDCTL_DSP_SETFRAGMENT");
        ON_ERROR;
    }

    // Sample format

    int fmt = AFMT_S16_NE;

    if (ioctl(_fd, SNDCTL_DSP_SETFMT, &fmt) == -1) {
        perror("SNDCTL_DSP_SETFMT");
        ON_ERROR;
    }

    if (fmt != AFMT_S16_NE) {
        fprintf(stderr, "The device does not support AFMT_S16_NE\n");
        ON_ERROR;
    }

    // Channel count

    int channels = config.channels;

    if (ioctl(_fd, SNDCTL_DSP_CHANNELS, &channels) == -1) {
        perror("SNDCTL_DSP_CHANNELS");
        ON_ERROR;
    }

    if (channels != config.channels) {
        fprintf(stderr, "The device does not support stereo output\n");
        ON_ERROR;
    }

    // Sample rate

    int sample_rate = config.sample_rate;
    if (ioctl(_fd, SNDCTL_DSP_SPEED, &sample_rate) == -1) {
        perror("SNDCTL_DSP_SPEED");
        ON_ERROR;
    }

    config.sample_rate = sample_rate;

    config.current_audio_driver = "OSS";

#ifdef ENABLE_REALTIME
    config.current_audio_driver_wants_realtime = 1;
#endif

    return 0;
}

int
OSSAudioDriver::write(float *buffer, int frames)
{
    Configuration &config = Configuration::get();

    int p = 0;
	int i;
	signed short _tmp;
	
	if (_outputBufferFrames < (unsigned int)frames) {
		_outputBufferFrames = (unsigned int)frames;
		if (_outputBuffer) { free(_outputBuffer); }
		_outputBuffer = (unsigned char*)malloc(frames * 2 * config.channels);
	}
	
    for ( i = 0; i < (frames * config.channels); i++) {
		_tmp = (signed short) (buffer[i] * 30000);
		_outputBuffer[p++] = (unsigned char) (_tmp & 0xff);
		_outputBuffer[p++] = (unsigned char) ((_tmp >> 8) & 0xff);
    }

    if ((::write(_fd, _outputBuffer, frames * 2)) != frames * 2) {
		perror("Error writing to OSS audio device");
		return -1;
	}
	
	return 0;
}

void OSSAudioDriver::close()
{
    if (_fd != -1) {
	   ::close(_fd);
       _fd = -1;
    }
    free(_outputBuffer);
    _outputBuffer = nullptr;
    _outputBufferFrames = 0;
}

#endif


class AudioDriver * CreateOSSAudioDriver()
{
#ifdef WITH_OSS
    return new OSSAudioDriver();
#else
    return 0;
#endif
}
