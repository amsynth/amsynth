/* Synth--
 * (c) 2001 Nick Dowell
 **/

#ifndef _ALSA_AUDIO_DRIVER_H
#define _ALSA_AUDIO_DRIVER_H

#include "AudioDriver.h"

#ifdef _ALSA
#include <alsa/asoundlib.h>
#endif

class ALSAAudioDriver:public AudioDriver {

  public:
    ALSAAudioDriver();
    virtual ~ ALSAAudioDriver();
    int open();
	int open( Config & config ){ 
		return -1; 
	};
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
};


#endif				// _ALSA_AUDIO_DRIVER_H
