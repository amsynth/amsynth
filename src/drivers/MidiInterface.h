/* Synth--
 * (c) 2001-2006 Nick Dowell
 **/

#ifndef _MidiInterface_h
#define _MidiInterface_h

#include "../Config.h"
#include "../Thread.h"

class MidiStreamReceiver
{
public:
	MidiStreamReceiver() : _midiIface(0) {}
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
    virtual ~MidiInterface();
	
	virtual int open(Config&);
	virtual void close();
	
	virtual void SetMidiStreamReceiver(MidiStreamReceiver* in);

    virtual int write_cc(unsigned int channel, unsigned int param, unsigned int value);
	
	virtual void Stop();
	
protected:
	virtual void ThreadAction();

	MidiStreamReceiver* _handler;
	
private:
	class MidiDriver * midi;
	unsigned char *_buffer;
};

#endif
