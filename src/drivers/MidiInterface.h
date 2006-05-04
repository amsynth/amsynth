/* Synth--
 * (c) 2001-2006 Nick Dowell
 **/

#include "ALSAMidiDriver.h"
#include "OSSMidiDriver.h"
#include "../Config.h"

class MidiInterface {
  public:
    MidiInterface();
    ~MidiInterface();
    int open( Config & config );
    void close();
    int read(unsigned char *bytes, unsigned maxBytes);
    int write_cc(unsigned int channel, unsigned int param, unsigned int value);
  private:
     MidiDriver * midi;
};
