/*
 *  OSSAudioDriver.cc
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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include <cstddef>

#include "OSSAudioDriver.h"

#ifdef WITH_OSS
#include "../Configuration.h"
#include "AudioDriver.h"

#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <unistd.h>

using namespace std;


class OSSAudioDriver : public AudioDriver
{
  public:
    OSSAudioDriver();
    int open();
    void close();
    int write( float *buffer, int frames );
    int setChannels( int channels );
    int setRate( int rate );
    int getRate(){ return rate_; };
    int setRealtime();

  private:
    int dsp_handle_, rate_, stereo_, format_, channels_;
    unsigned char *_outputBuffer;
    unsigned int _outputBufferFrames;
};

int
OSSAudioDriver::write(float *buffer, int frames)
{
    int p = 0;
	int i;
	signed short _tmp;
	
	if (_outputBufferFrames < (unsigned int)frames) {
		_outputBufferFrames = (unsigned int)frames;
		if (_outputBuffer) { free(_outputBuffer); }
		_outputBuffer = (unsigned char*)malloc(frames * 2 * channels_);
	}
	
    for ( i = 0; i < (frames * channels_); i++) {
		_tmp = (signed short) (buffer[i] * 30000);
		_outputBuffer[p++] = (unsigned char) (_tmp & 0xff);
		_outputBuffer[p++] = (unsigned char) ((_tmp >> 8) & 0xff);
    }

    if ((::write(dsp_handle_, _outputBuffer, frames*2 )) != frames * 2) {
		perror("<OSSAudioDriver> error writing to dsp_handle_");
		return -1;
	}
	
	return 0;
}

int OSSAudioDriver::open()
{
    Configuration & config = Configuration::get();

    if ((dsp_handle_ =::open(config.oss_audio_device.c_str(), O_WRONLY)) == -1){
		cout << "<OSSAudioDriver> error: could not open dsp device " 
			<< config.oss_audio_device << endl;
	return -1;
   }

    //  setRealtime();

    // set sample format (number of bits)
    if (ioctl(dsp_handle_, SNDCTL_DSP_SETFMT, &format_) == -1)
		perror("ioctl format");
    setChannels( config.channels );
    setRate( config.sample_rate );
	config.sample_rate = getRate();
	
	config.current_audio_driver = "OSS";
#ifdef ENABLE_REALTIME
	config.current_audio_driver_wants_realtime = 1;
#endif
	
    return 0;
}

int OSSAudioDriver::setChannels(int channels)
{
    channels_ = channels;
    switch (channels_) {
    case 1:
	stereo_ = 0;
	break;
    case 2:
	stereo_ = 1;
	break;
    default:
	return -1;
    }

    if (ioctl(dsp_handle_, SNDCTL_DSP_STEREO, &stereo_) == -1) {
	perror("ioctl stereo");
	return -1;
    }
    return 0;
}

int OSSAudioDriver::setRate(int rate)
{
    rate_ = rate;
    // set sampling rate
    if (ioctl(dsp_handle_, SNDCTL_DSP_SPEED, &rate_) == -1) {
		perror("ioctl sample rate");
		return -1;
    }
    return 0;
}

int OSSAudioDriver::setRealtime()
{
    /* set fragment size... (WHAT IS IT?!?)
     * fragment size 0xMMMMSSSS
     * fragment size (bytes) = 2^SSSS eg SSSS=0008 = fragment size of 256
     * experimentation on my athlon (500) + SBLive gives:
     * 0007 (128 bytes)= smallest usable (no - slight glitches)
     * 0008 (256 bytes)= good latency
     * 0009 (512 bytes)= i can notice the delay here..
     * !OSS Documentation warns against using frag < 0008
     * apparently its best to fill the fragment in one go with each write() 
     * 2^MMMM = number of fragments. min=2, 0x7fff = no limit
     * 0001 (2) fragments @ 256bytes = disastrous
     * 0002 (4) fragments @ 256bytes = good (fine?)  [1024 bytes total buffer]
     * 0003 (8) fragments @ 256bytes = ok (latency?) -problems under ALSA-OSS
     * 0004 (16) frags @128bytes = good (both)         [2048 bytes total buffer]
     * unlimited              = disastrous for latency..
     */
    int frag = 0x00060008;
    if (ioctl(dsp_handle_, SNDCTL_DSP_SETFRAGMENT, &frag) == -1) {
	perror("err: ioctl fragment");
	return -1;
    }
    return 0;
}

void OSSAudioDriver::close()
{
    if (dsp_handle_ != -1) {
	::close(dsp_handle_);
	free(_outputBuffer);
	_outputBuffer = NULL;
	_outputBufferFrames = 0;
    }
}

OSSAudioDriver::OSSAudioDriver()
{
    rate_ = 0;
    stereo_ = 1;
    format_ = AFMT_S16_LE;
    dsp_handle_ = -1;
    _outputBuffer = NULL;
    _outputBufferFrames = 0;
}

#endif


class AudioDriver * CreateOSSAudioDriver()
{
#ifdef WITH_OSS
    return new OSSAudioDriver();
#else
    return NULL;
#endif
}
