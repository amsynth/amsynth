/* amSynth
 * (c) 2001,2002 Nick Dowell
 **/

#ifndef _ALSA_AUDIO_DRIVER_H
#define _ALSA_AUDIO_DRIVER_H

#include "AudioDriver.h"

#ifdef with_alsa
#include <alsa/asoundlib.h>
#endif

#define BUFSIZE 64

class ALSAAudioDriver:public AudioDriver {

  public:
    ALSAAudioDriver();
    virtual ~ ALSAAudioDriver();
    int open(){ return -1; };
	int open( Config & config );
    void close();
    int write(float *buffer, int frames);
    int setChannels(int channels);
    int setRate(int rate);
    int setRealtime();

  private:
    int _dsp_handle;
    int _rate;
    int _channels;
    int _format;
	unsigned char *audiobuf;
	
	Config		*config;
#ifdef with_alsa
	snd_pcm_t *playback_handle;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_sw_params_t *sw_params;
#endif
};


#endif				// with_alsa_AUDIO_DRIVER_H
