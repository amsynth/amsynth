/* amSynth
 * (c) 2001,2002 Nick Dowell
 */
#include "AudioInterface.h"

int
AudioInterface::setRealtime()
{
	if (driver)
		return driver->setRealtime();
	else
		return -1;
}

int
AudioInterface::setRate(int rate)
{
	if (driver)
		return driver->setRate(rate);
	else
		return -1;
}

int
AudioInterface::setChannels(int channels)
{
	if (driver)
		return driver->setChannels(channels);
	else
		return -1;
}

int
AudioInterface::open( Config & config )
{
	if (driver) delete driver;
	driver = 0;
	
	// auto-select - try all drivers, best first
	if( config.audio_driver == "auto" || config.audio_driver == "AUTO" )
	{
		
		// try ALSA-MMAP
		driver = new ALSAmmapAudioDriver;
		if ( driver->open( config ) == 0 )
		{
#ifdef _DEBUG
			cout << "<AudioInterface> opened ALSA-MMAP AudioDriver" << endl;
#endif
			return 0;
		}
		delete driver; driver = 0;
		
		// try ALSA
		driver = new ALSAAudioDriver;
		if ( driver->open( config ) == 0 )
		{
#ifdef _DEBUG
			cout << "<AudioInterface> opened ALSA AudioDriver" << endl;
#endif
			return 0;
		}
		delete driver; driver = 0;
		
		//try OSS
		driver = new OSSAudioDriver;
		if ( driver->open( config ) == 0 )
		{
#ifdef _DEBUG
			cout << "<AudioInterface> opened OSS AudioDriver" << endl;
#endif
			return 0;
		}
		delete driver; driver = 0;
	} 
	else if( config.audio_driver == "oss" || config.audio_driver == "OSS" )
	{
		driver = new OSSAudioDriver;
		if ( driver->open( config ) == 0 )
		{
#ifdef _DEBUG
			cout << "<AudioInterface> opened OSS AudioDriver" << endl;
#endif
			return 0;
		} 
		else
		{
			delete driver;
			driver = 0;
		}
	} 
	else if( config.audio_driver == "alsa" || config.audio_driver == "ALSA" )
	{
		driver = new ALSAAudioDriver;
		if ( driver->open( config ) == 0 )
		{
#ifdef _DEBUG
			cout << "<AudioInterface> opened ALSA AudioDriver" << endl;
#endif
			return 0;
		} 
		else
		{
			delete driver;
			driver = 0;
		}
	}
	else if( config.audio_driver == "alsa-mmap" || config.audio_driver == "ALSA-MMAP" )
	{
		driver = new ALSAmmapAudioDriver;
		if ( driver->open( config ) == 0 )
		{
#ifdef _DEBUG
			cout << "<AudioInterface> opened ALSA-MMAP AudioDriver" << endl;
#endif
			return 0;
		}
		else
		{
			delete driver;
			driver = 0;
		}
	}
	cerr << "error: could not find \"" << config.audio_driver << "\" audio driver\n";
	return -1;
}

void
AudioInterface::close()
{
	if (driver) driver->close();
}

AudioInterface::AudioInterface()
{
	_no_of_drivers = 2;
	_drivers = new int[_no_of_drivers];
	_drivers[0] = AUDIO_DRIVER_ALSA;
	_drivers[1] = AUDIO_DRIVER_OSS;
	driver = 0;
}

AudioInterface::~AudioInterface()
{
	if (driver) delete driver;
}
