/* amSynth
 * (c) 2001-2006 Nick Dowell
 **/

#include "ALSAAudioDriver.h"
#include <iostream>

using namespace std;

int
ALSAAudioDriver::write(float *buffer, int frames)
{
#ifdef with_alsa
	int i,p,tmp;
//	if (!audiobuf)
	audiobuf = (unsigned char*)malloc((frames*_channels*2));
	
	p=0;
	for( i=0; i<(frames); i++ ){
		tmp = (int)((float)32767*buffer[i]);
		audiobuf[p++] = (unsigned char) (tmp & 0xff);
		audiobuf[p++] = (unsigned char) ((tmp >> 8) & 0xff);
	}
	
	while( snd_pcm_writei( playback_handle, audiobuf, frames/2 ) < 0){
		snd_pcm_prepare( playback_handle );
		config->xruns++;
//		cerr << "buffer underrun - please set realtime priority\n";
	}
	free( audiobuf );
	return 0;
#else
	return -1;
#endif
}

int 
ALSAAudioDriver::open( Config & config )
{
	if (playback_handle != NULL) return 0;

	_channels = config.channels;
	_rate = config.sample_rate;
#ifdef with_alsa
	if(snd_pcm_open(&playback_handle, config.alsa_audio_device.c_str(), SND_PCM_STREAM_PLAYBACK, 0)<0){
		cerr << "ALSA: cannot open audio device " << config.alsa_audio_device << endl;
		return -1;
	}
    snd_pcm_hw_params_alloca( &hw_params );
    snd_pcm_hw_params_any( playback_handle, hw_params );
    snd_pcm_hw_params_set_access( playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED );
    snd_pcm_hw_params_set_format( playback_handle, hw_params, SND_PCM_FORMAT_S16_LE );
    snd_pcm_hw_params_set_rate_near( playback_handle, hw_params, _rate, 0 );
    snd_pcm_hw_params_set_channels( playback_handle, hw_params, _channels );
	snd_pcm_hw_params_set_periods( playback_handle, hw_params, 32, 0 );
	snd_pcm_hw_params_set_period_size( playback_handle, hw_params, BUFSIZE, 0 );
    snd_pcm_hw_params( playback_handle, hw_params );
	
	config.sample_rate = snd_pcm_hw_params_get_rate( hw_params, 0 );
	config.current_audio_driver = "ALSA";
	
	this->config = &config;

	return 0;
#else
	return -1;
#endif
}

void ALSAAudioDriver::close()
{
#ifdef with_alsa
	if (playback_handle != NULL) snd_pcm_close (playback_handle);
	playback_handle = NULL;
#endif
}

int ALSAAudioDriver::setChannels(int channels)
{
  // WRITE ME!
  _channels = channels;
  return 0;
}


int ALSAAudioDriver::setRate(int rate)
{
  // WRITE ME!
  _rate = rate;
  return 0;
}

int ALSAAudioDriver::setRealtime()
{
  // WRITE ME!
  return 0;
}

ALSAAudioDriver::ALSAAudioDriver()
:	playback_handle (NULL)
{
}

ALSAAudioDriver::~ALSAAudioDriver()
{
  close();
}
