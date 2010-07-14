/* amSynth
 * (c) 2001-2003 Nick Dowell
 **/
#ifdef with_oss
#include <sys/soundcard.h>
#endif

#include <sys/ioctl.h>
#include <fcntl.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "OSSAudioDriver.h"
#include "../Config.h"

int
OSSAudioDriver::write(float *buffer, int frames)
{
#ifdef with_oss
    int p = 0;
	int i;
	signed short _tmp;
	
	if (_outputBufferFrames < frames) {
		_outputBufferFrames = frames;
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
#else
    UNUSED_PARAM(buffer);
    UNUSED_PARAM(frames);
    return -1;
#endif
}

int OSSAudioDriver::open( Config & config )
{
#ifdef with_oss
#ifdef _DEBUG
	cout << "<OSSAudioDriver::open()>" << endl;
#endif
	bufsize_ = config.buffer_size;
    if ((dsp_handle_ =::open(config.oss_audio_device.c_str(), O_WRONLY)) == -1){
		cout << "<OSSAudioDriver> error: could not open dsp device " 
			<< config.oss_audio_device << endl;
	return -1;
   }
#ifdef _DEBUG
    cout << "<OSSAudioDriver> opened OSS dsp output :-)" << endl;
#endif

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
	
	this->config = &config;
	
    return 0;
#else
    UNUSED_PARAM(config);
    return -1;
#endif
}

int OSSAudioDriver::setChannels(int channels)
{
#ifdef with_oss
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
#else
    UNUSED_PARAM(channels);
    return -1;
#endif
}

int OSSAudioDriver::setRate(int rate)
{
#ifdef with_oss
    rate_ = rate;
    // set sampling rate
    if (ioctl(dsp_handle_, SNDCTL_DSP_SPEED, &rate_) == -1) {
		perror("ioctl sample rate");
		return -1;
    }
#ifdef _DEBUG
	cout << "<OSSAudioDriver::setRate()> rate set to " << rate << endl;
#endif //_DEBUG
    return 0;
#else // not with_oss
    UNUSED_PARAM(rate);
    return -1;
#endif //with_oss
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
#ifdef with_oss
    int frag = 0x00060008;
    if (ioctl(dsp_handle_, SNDCTL_DSP_SETFRAGMENT, &frag) == -1) {
	perror("err: ioctl fragment");
	return -1;
    }
    return 0;
#else
    return -1;
#endif
}

void OSSAudioDriver::close()
{
#ifdef with_oss
    if (dsp_handle_ != -1) {
	::close(dsp_handle_);
	free(_outputBuffer);
	_outputBuffer = NULL;
	_outputBufferFrames = 0;
#ifdef _DEBUG
	cout << "<OSSAudioDriver> closed OSS dsp device." << endl;
#endif
    }
#endif
}

OSSAudioDriver::OSSAudioDriver()
{
#ifdef with_oss
    rate_ = 0;
    stereo_ = 1;
    format_ = AFMT_S16_LE;
    dsp_handle_ = -1;
    _outputBuffer = NULL;
    _outputBufferFrames = 0;
#endif
}

OSSAudioDriver::~OSSAudioDriver()
{
    close();
}
