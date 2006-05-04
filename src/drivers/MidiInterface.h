/* Synth--
 * (c) 2001-2006 Nick Dowell
 **/

#include "ALSAMidiDriver.h"
#include "OSSMidiDriver.h"

#include "../Config.h"
#include "../Thread.h"

class MidiStreamReceiver
{
public:
	virtual ~MidiStreamReceiver() {}
	virtual void HandleMidiData(unsigned char* bytes, unsigned numBytes) = 0;
	
	// just a kludge for the time being so that we can still do MIDI out:
	virtual void SetMidiInterface(class MidiInterface* in) { _midiIface = in; }
protected:
	class MidiInterface *_midiIface;
};


class MidiInterface : public Thread
{
public:
    MidiInterface();
    ~MidiInterface();
	
	int open(Config&);
	void close();
	
	void SetMidiStreamReceiver(MidiStreamReceiver* in);

    int write_cc(unsigned int channel, unsigned int param, unsigned int value);
	
	virtual void Stop();
	
protected:
	virtual void ThreadAction();
	
private:
     MidiDriver * midi;
	 unsigned char *_buffer;
	 MidiStreamReceiver* _handler;
};
