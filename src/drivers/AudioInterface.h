/*
 *  AudioInterface.h
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

#ifndef _AUDIO_INTERFACE_H
#define _AUDIO_INTERFACE_H

class AudioInterface
{
public:

    AudioInterface() : _driver(0) {}
    ~AudioInterface() { close(); }
  /** 
   * Attempts to open an audio driver. It will try each driver in turn until
   * it finds one which works.
   * 
   * @return returns 0 on success, -1 on failure
   */
	  int  open(class Config &);
    void close();
    int  write(float *buffer, int frames);
    int  setRealtime();

private:

    class AudioDriver *_driver;
};

#endif
