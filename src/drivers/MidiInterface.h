/* Synth--
 * (c) 2001 Nick Dowell
 **/

#include "ALSAMidiDriver.h"
#include "OSSMidiDriver.h"
#include "../Config.h"

/**
 * \class MidiInterface
 * \brief An all-singing all-dancing midi interface driver.
 */
class MidiInterface {
  public:
    MidiInterface();
    ~MidiInterface();
    int open( Config & config );
    void close();
    int read(unsigned char *buffer);

  private:
     MidiDriver * midi;
};
