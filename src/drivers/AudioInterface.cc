/* amSynth
 * (c) 2001 Nick Dowell
 */
#include "AudioInterface.h"

int
AudioInterface::write(float *buffer, int frames)
{
    return driver->write(buffer, frames);
}

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
    // try ALSA
    driver = new ALSAAudioDriver;
    if (driver->open( config ) == 0) {
#ifdef _DEBUG
	cout << "<AudioInterface> opened ALSA AudioDriver" << endl;
#endif
	return 0;
    }
    //try OSS
    delete driver;
    driver = new OSSAudioDriver;
    if (driver->open( config ) == 0) {
#ifdef _DEBUG
	cout << "<AudioInterface> opened OSS AudioDriver" << endl;
#endif
	return 0;
    }
    delete driver;
    return -1;
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
