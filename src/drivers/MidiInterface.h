/* Synth--
 * (c) 2001 Nick Dowell
 **/

#include "ALSAMidiDriver.h"
#include "OSSMidiDriver.h"

/**
 * \class MidiInterface
 * \brief An all-singing all-dancing midi interface driver.
 */
class MidiInterface {
  public:
    MidiInterface();
    ~MidiInterface();
    int open( string device );
    void close();
    int read(unsigned char *buffer);

  private:
     MidiDriver * midi;
};
