/* amSynth
 * (c) 2001,2002 Nick Dowell
 **/

#include "ALSAAudioDriver.h"

int
ALSAAudioDriver::write(float *buffer, int frames)
{
	int ret, i, p, ch;
	int intbuf[frames*_channels];
	for( i=0; i<(frames*_channels); i++ )
		intbuf[i] = (int)(buffer[i]*32768);
	
	while( snd_pcm_writei( playback_handle, intbuf, frames ) < 0){
		snd_pcm_prepare( playback_handle );
		cerr << "buffer underrun - please set realtime priority\n";
	}
	return 0;
}

int 
ALSAAudioDriver::open( Config & config )
{
	_channels = config.channels;
	_rate = config.sample_rate;
#ifdef _ALSA
	char *pcm_name = "plughw:0,0";
	if(snd_pcm_open(&playback_handle, pcm_name, SND_PCM_STREAM_PLAYBACK, 0)<0){
		cerr << "ALSA: cannot open audio device " << pcm_name << endl;
		return -1;
	}
    snd_pcm_hw_params_alloca( &hw_params );
    snd_pcm_hw_params_any( playback_handle, hw_params );
    snd_pcm_hw_params_set_access( playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED );
    snd_pcm_hw_params_set_format( playback_handle, hw_params, SND_PCM_FORMAT_S16_LE );
    snd_pcm_hw_params_set_rate_near( playback_handle, hw_params, _rate, 0 );
    snd_pcm_hw_params_set_channels( playback_handle, hw_params, 2 );
	snd_pcm_hw_params_set_periods( playback_handle, hw_params, 32, 0 );
	snd_pcm_hw_params_set_period_size( playback_handle, hw_params, BUFSIZE, 0 );
    snd_pcm_hw_params( playback_handle, hw_params );
/*    snd_pcm_sw_params_alloca( &sw_params );
    snd_pcm_sw_params_current( playback_handle, sw_params );
    snd_pcm_sw_params_set_avail_min( playback_handle, sw_params, BUFSIZE );
    snd_pcm_sw_params( playback_handle, sw_params );
	*/
//	snd_pcm_prepare( playback_handle );
	config.sample_rate = snd_pcm_hw_params_get_rate( hw_params, 0 );

	config.audio_driver = "ALSA";
	return 0;
#else
	return -1;
#endif
}

void ALSAAudioDriver::close()
{
#ifdef _ALSA
	snd_pcm_close( playback_handle );
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
{
  // WRITE ME!
}

ALSAAudioDriver::~ALSAAudioDriver()
{
  close();
}
