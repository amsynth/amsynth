/* amSynth
 * (c) 2001,2002 Nick Dowell
 **/

#ifndef _ALSA_MMAP_AUDIO_DRIVER_H
#define _ALSA_MMAP_AUDIO_DRIVER_H

#include "AudioDriver.h"

#ifdef _ALSA
#include <alsa/asoundlib.h>
#endif

#define BUFSIZE 64

class ALSAmmapAudioDriver : public AudioDriver {

  public:
    ALSAmmapAudioDriver();
    virtual ~ ALSAmmapAudioDriver();
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
#ifdef _ALSA
	snd_pcm_t *playback_handle;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_sw_params_t *sw_params;
	int xrun_recovery();
	int	err;
	unsigned periods;

#endif
};


#endif				// _ALSA_MMAP_AUDIO_DRIVER_H
