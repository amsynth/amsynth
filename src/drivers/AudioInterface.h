// (c) 2001 Nick Dowell

#ifndef _AUDIO_INTERFACE_H
#define _AUDIO_INTERFACE_H

#include "AudioDriver.h"
#include "ALSAAudioDriver.h"
#include "ALSAmmapAudioDriver.h"
#include "OSSAudioDriver.h"
#include <iostream.h>

#define AUDIO_DRIVER_NONE 0
#define AUDIO_DRIVER_ALSA 1
#define AUDIO_DRIVER_OSS  2

/**
 * @class AudioInterface
 * @brief An all-singing all-dancing audio interface driver.
 *
 * The AudioInterface is a high-level audio driver, in that it can use any
 * system-level drivers which may be on the host. New system-level drivers
 * can be added by creating a new AudioDriver.. AudioInterface also performs
 * any sample format conversion; at the moment it accepts floating point
 * format samples.
 */
class AudioInterface {

  public:
    AudioInterface();
    ~AudioInterface();
  /** 
   * Attempts to open an audio driver. It will try each driver in turn until
   * it finds one which works.
   * 
   * @return returns 0 on success, -1 on failure
   */
    int open(){
		return -1;
	};
	int open( Config & config );
  /** 
   * Closes the active driver.
   * 
   */
    void close();
  /** 
   * Writes the audio data in buffer to the driver, performing any sample 
   * conversion first obviously.
   * 
   * @return 0 on success(all samples were written to the device), 
   * -1 on failure.
   */
    int write(float *buffer, int frames)
	{ return driver->write( buffer, frames ); };
  /** 
   * not implemented yet
   * 
   * 
   * @return 
   */
    int read();
  /** 
   * Sets the sample rate to be used.
   * 
   * 
   * @return 0 on success, -1 on failure.
   */
    int setRate(int rate);
  /** 
   * Sets the number of channels for the output.
   * 
   * 
   * @return 0 on success, -1 on failure.
   */
    int setChannels(int channels);
  /** 
   * Sets the precision (sample width) for the output. Default is 16 bits.
   * 
   * 
   * @return 0 on success, -1 on failure.
   */
    int setPrecision();
  /** 
   * Configures the device for (near) realtime / low latency response. 
   * (soon to be deprecated)
   * 
   * @return 0 on success, -1 on failure.
   */
    int setRealtime();
  /**
   * Sets the number and size of fragments used in the audio driver's buffers.
   *
   * @param frags the number of fragments to use. 2^frags fragments will be 
   *        used. eg frags = 8 corresponds to 256 fragments.
   * @param fragSize the size of each fragment. again, sets the size to 
   *        2^fragSize.
   * @return 0 on success, -1 on failure.
   */
    int setFragment(int frags, int fragSize) {
	_frags = frags;
	_fragSize = fragSize;
	return 0;
    };
  private:
    AudioDriver * driver;
    int _active_driver, *_drivers, _no_of_drivers;
    int _rate, _channels, _width, _frags, _fragSize;
};

#endif
