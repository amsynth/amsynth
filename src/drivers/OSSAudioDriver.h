/*
 *  OSSAudioDriver.h
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

#ifndef _OSS_AUDIO_DRIVER_H
#define _OSS_AUDIO_DRIVER_H

#include "AudioDriver.h"

class OSSAudioDriver:public AudioDriver {

  public:
    OSSAudioDriver();
	int open(){
		return -1;
	};
    int open( Config & config );
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
	Config *config;
};


#endif				// _OSS_AUDIO_DRIVER_H
