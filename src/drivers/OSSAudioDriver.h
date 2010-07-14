/* amSynth
 * (c) 2001-2003 Nick Dowell
 **/

#ifndef _OSS_AUDIO_DRIVER_H
#define _OSS_AUDIO_DRIVER_H

#include "AudioDriver.h"

class OSSAudioDriver:public AudioDriver {

  public:
    OSSAudioDriver();
    virtual ~ OSSAudioDriver();
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
    int dsp_handle_, rate_, stereo_, format_, channels_, bufsize_;
    unsigned char *_outputBuffer;
    unsigned int _outputBufferFrames;
	Config *config;
};


#endif				// _OSS_AUDIO_DRIVER_H
