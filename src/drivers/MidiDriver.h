/* Synth--
 * (c) Nick Dowell
 **/

#ifndef _MIDI_DRIVER_H
#define _MIDI_DRIVER_H

#define MIDI_BUF_SIZE 1

#include <string>

/** \class MidiDriver
 *  \brief a Generic MIDI Interface
 *
 * an abstraction of Midi driver interfaces, to allow use of all? (sane) MIDI
 * drivers. Only the simplest functions are defined.
 */
class MidiDriver {
  public:
    // read() returns the number of bytes succesfully read. numbers < 0 
    // generally indicate failure...
    virtual int read(unsigned char *midi_event_buffer) = 0;
    virtual int open(string device) = 0;
    virtual int close() = 0;
};

#endif
