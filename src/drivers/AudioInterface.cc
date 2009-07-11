/* amSynth
 * (c) 2001,2002 Nick Dowell
 */
#include "AudioInterface.h"

#define SAFE_DELETE(o) if (o) { delete o; o = 0; }

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

bool
try_driver(AudioDriver * ad, Config & cfg)
{
	if (ad->open(cfg) != 0)
		return false;
	
	const size_t numFrames = 1024;
	void *buffer = calloc(numFrames, 8);
	int write_res = ad->write((float *)buffer, numFrames);
	free(buffer);
	
	return write_res == 0;
}

int
AudioInterface::open( Config & config )
{
	SAFE_DELETE(driver);
	
	// try ALSA-MMAP
	if (config.audio_driver == "auto" ||
		config.audio_driver == "alsa-mmap")
	{	
		driver = new ALSAmmapAudioDriver;
		if (try_driver(driver, config))
		{
			if (config.debug_drivers)
				cout << "<AudioInterface> opened ALSA-MMAP AudioDriver" << endl;
			return 0;
		}
	}
	
	SAFE_DELETE(driver);
	
	// try ALSA
	if (config.audio_driver == "auto" ||
		config.audio_driver == "alsa")
	{	
		driver = new ALSAAudioDriver;
		if (try_driver(driver, config))
		{
			if (config.debug_drivers)
				cout << "<AudioInterface> opened ALSA AudioDriver" << endl;
			return 0;
		}
	}
	
	SAFE_DELETE(driver);
	
	//try OSS
	if (config.audio_driver == "auto" ||
		config.audio_driver == "oss")
	{	
		driver = new OSSAudioDriver;
		if (try_driver(driver, config))
		{
			if (config.debug_drivers)
				cout << "<AudioInterface> opened OSS AudioDriver" << endl;
			return 0;
		}
	}
	
	SAFE_DELETE(driver);
	
	cerr << "error: could not start audio driver: " << config.audio_driver << "\n";
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
	SAFE_DELETE(driver);
}
