/* Synth--
 * (c) 2001 Nick Dowell
 **/

#include "ALSAAudioDriver.h"

int
ALSAAudioDriver::write(float *buffer, int frames)
{
	int outBuf[ frames*_channels ];
	for ( int i=0; i<(frames*_channels); i++) outBuf[i] = (buffer[i] * 30000);

	int res=snd_pcm_writei( playback_handle, outBuf, frames );
	if( res=-32 )
		// buffer underrun - just ignore
		return frames;
	if( res<0 ){
		cerr << "Error writing to alsa audio: " << res << "\n";
		cerr << "EBADFD:" << EBADFD << ", EPIPE:" << EPIPE 
			<< ", ESTRPIPE:" << ESTRPIPE << endl;
		return -1;
	}
	return res;
}

int ALSAAudioDriver::open( Config & config )
{
	_channels = config.channels;
	_rate = config.sample_rate;
#ifdef _ALSA
	char *pcm_name = "hw:0";
	if(snd_pcm_open(&playback_handle, pcm_name, SND_PCM_STREAM_PLAYBACK, 0)<0){
		fprintf (stderr, "cannot open audio device %s\n", pcm_name);
		return -1;
	}
    snd_pcm_hw_params_alloca( &hw_params );
    snd_pcm_hw_params_any( playback_handle, hw_params );
    snd_pcm_hw_params_set_access( playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED );
    snd_pcm_hw_params_set_format( playback_handle, hw_params, SND_PCM_FORMAT_S16 );
    snd_pcm_hw_params_set_rate_near( playback_handle, hw_params, _rate, 0 );
    snd_pcm_hw_params_set_channels( playback_handle, hw_params, _channels );
    snd_pcm_hw_params_set_periods( playback_handle, hw_params, 8, 0 );
    snd_pcm_hw_params_set_period_size( playback_handle, hw_params, BUFSIZE, 0 );
    snd_pcm_hw_params( playback_handle, hw_params );
    snd_pcm_sw_params_alloca( &sw_params );
    snd_pcm_sw_params_current( playback_handle, sw_params );
    snd_pcm_sw_params_set_avail_min( playback_handle, sw_params, BUFSIZE );
    snd_pcm_sw_params( playback_handle, sw_params );
	
	snd_pcm_prepare( playback_handle );
	config.sample_rate = snd_pcm_hw_params_get_rate( hw_params, 0 );
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
