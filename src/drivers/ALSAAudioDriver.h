/* Synth--
 * (c) 2001 Nick Dowell
 **/

#ifndef _ALSA_AUDIO_DRIVER_H
#define _ALSA_AUDIO_DRIVER_H

#include "AudioDriver.h"

#ifdef _ALSA
#include <alsa/asoundlib.h>
#endif

#define BUFSIZE 256

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
#ifdef _ALSA
	snd_pcm_t *playback_handle;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_sw_params_t *sw_params;
#endif
};


#endif				// _ALSA_AUDIO_DRIVER_H
