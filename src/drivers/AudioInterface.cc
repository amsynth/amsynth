/* amSynth
 * (c) 2001,2002 Nick Dowell
 */
#include "AudioInterface.h"

int
AudioInterface::setRealtime()
{
    return driver->setRealtime();
}

int
AudioInterface::setRate(int rate)
{
    return driver->setRate(rate);
}

int
AudioInterface::setChannels(int channels)
{
    return driver->setChannels(channels);
}

int
AudioInterface::open( Config & config )
{
	if( config.audio_driver == "auto" || config.audio_driver == "AUTO" ){
		// try ALSA
		driver = new ALSAAudioDriver;
		if ( driver->open( config ) == 0 ) {
#ifdef _DEBUG
			cout << "<AudioInterface> opened ALSA AudioDriver" << endl;
#endif
			return 0;
		}
		//try OSS
		delete driver;
		driver = new OSSAudioDriver;
		if ( driver->open( config ) == 0 ) {
#ifdef _DEBUG
			cout << "<AudioInterface> opened OSS AudioDriver" << endl;
#endif
			return 0;
		}
		delete driver;
		return -1;
	} 
	
	if( config.audio_driver == "oss" || config.audio_driver == "OSS" ){
		driver = new OSSAudioDriver;
		if ( driver->open( config ) == 0 ) {
#ifdef _DEBUG
			cout << "<AudioInterface> opened OSS AudioDriver" << endl;
#endif
			return 0;
		} else {
			delete driver;
			return -1;
		}
	} 
	
	if( config.audio_driver == "alsa" || config.audio_driver == "ALSA" ){
		driver = new ALSAAudioDriver;
		if ( driver->open( config ) == 0 ) {
#ifdef _DEBUG
			cout << "<AudioInterface> opened ALSA AudioDriver" << endl;
#endif
			return 0;
		} else {
			delete driver;
			return -1;
		}
	}
}

void
AudioInterface::close()
{
    driver->close();
}

AudioInterface::AudioInterface()
{
    _no_of_drivers = 2;
    _drivers = new int[_no_of_drivers];
    _drivers[0] = AUDIO_DRIVER_ALSA;
    _drivers[1] = AUDIO_DRIVER_OSS;
}

AudioInterface::~AudioInterface()
{
    delete driver;
}