/* Synth--
 * (c) 2001 Nick Dowell
 **/

#ifndef _OSS_MIDI_DRIVER_H
#define _OSS_MIDI_DRIVER_H

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#ifdef _DEBUG
#include <iostream>
#endif

#include "MidiDriver.h"

class OSSMidiDriver:public MidiDriver {

public:
  OSSMidiDriver();
  virtual ~OSSMidiDriver();
  int read(unsigned char *midi_event_buffer);
  int open(string device, string);
  int close();
  
private:
  int _oss_midi_fd;		// File descriptor for OSS midi device
  int opened;
};

#endif				// _ALSA_MIDI_DRIVER_H
