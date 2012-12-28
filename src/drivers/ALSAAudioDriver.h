/*
 *  ALSAAudioDriver.h
 *
 *  Copyright (c) 2001-2012 Nick Dowell
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

#ifndef _ALSA_AUDIO_DRIVER_H
#define _ALSA_AUDIO_DRIVER_H

#include "AudioDriver.h"

#ifdef WITH_ALSA
#define ALSA_PCM_OLD_HW_PARAMS_API
#define ALSA_PCM_OLD_SW_PARAMS_API
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
#ifdef WITH_ALSA
	snd_pcm_t *playback_handle;
    snd_pcm_hw_params_t *hw_params;
    snd_pcm_sw_params_t *sw_params;
#endif
};


#endif				// with_alsa_AUDIO_DRIVER_H
