/*
 *  AudioDriver.h
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

#ifndef _AUDIO_DRIVER_H
#define _AUDIO_DRIVER_H

#include "../Config.h"

// useful for avoiding compiler warnings
#define UNUSED_PARAM( x ) (void)x

using namespace std;

/** 
 * @class AudioDriver
 * @brief A generic audio driver interface
 *
 * An abstraction of audio driver interfaces, to allow any system-level
 * driver to be used..
 */
class AudioDriver
{
public:
	virtual ~AudioDriver () {}
	
  /** 
   * Opens the audio driver, ready for input.
   * @return 0 on success, -1 on error.
   */
    virtual int open()
    = 0;
	virtual int open( Config & config )
	= 0;
  /** 
   * Closes the audio driver.
   */
    virtual void close()
    = 0;
  /** 
   * Writes the data in buffer to the audio driver.
   * @param buffer pointer to the audio data to be written
   * @param frames number of frames to write
   * @return 0 on success, -1 on failure.
   */
    virtual int write(float *buffer, int frames)
    = 0;
  /** 
   * Configures the device for (near) realtime / low latency response.
   * (soon to be deprecated)
   * @return 0 on success, -1 on failure.
   */
    virtual int setRealtime()
    = 0;
  /** 
   * Set the number of audio channels for output.
   * @param channels the number of output channels
   * @return 0 on success, -1 on failure.
   */
    virtual int setChannels(int channels)
    = 0;
  /** 
   * Sets the output sampling rate.
   * @param rate the sampling rate.
   * @return 0 on success, -1 on failure.
   */
    virtual int setRate(int rate)
    = 0;
};


#endif				// _AUDIO_DRIVER_H
