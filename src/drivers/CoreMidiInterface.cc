/*
 *  CoreMidiInterface.cc
 *  amsynth
 */

#include "CoreMidiInterface.h"

#if (__APPLE__)

#include <CoreMIDI/MIDIServices.h>

class CoreMidiInterface : public MidiInterface
{
public:
	
	CoreMidiInterface();
	~CoreMidiInterface();
	
	virtual int open(Config&);
	virtual void close();

protected:
		
	static void midiNotifyProc (const MIDINotification*, void*);
	static void midiReadProc (const MIDIPacketList*, void*, void*);
	
	void HandleMidiPacketList(const MIDIPacketList*);
	
private:
	
	MIDIClientRef   m_clientRef;
	MIDIPortRef     m_inPort;
	MIDIEndpointRef m_currInput;
};

MidiInterface* CreateCoreMidiInterface() { return new CoreMidiInterface; }


CoreMidiInterface::CoreMidiInterface()
{
	MIDIClientCreate(CFSTR("amSynth"), midiNotifyProc, this, &m_clientRef);
	MIDIInputPortCreate(m_clientRef, CFSTR("Input port"), midiReadProc, this, &m_inPort);
}

CoreMidiInterface::~CoreMidiInterface()
{
	MIDIClientDispose(m_clientRef);
}

int
CoreMidiInterface::open(Config&)
{
	m_currInput = MIDIGetSource(0);
	if (m_currInput)
	{
		MIDIPortConnectSource(m_inPort, m_currInput, NULL);
		
		CFStringRef name;
		MIDIObjectGetStringProperty(m_currInput, kMIDIPropertyName, &name);
		CFShow(name);		
	}
	return (m_currInput) ? 0 : -1;
}

void
CoreMidiInterface::close()
{
	if (m_currInput) 
	{
		MIDIPortDisconnectSource(m_inPort, m_currInput);
		m_currInput = NULL;
	}
}

void
CoreMidiInterface::HandleMidiPacketList(const MIDIPacketList* pktlist)
{
	if (!_handler) return;
	
	for (int i=0; i<pktlist->numPackets; i++)
	{
		unsigned char* data = (unsigned char *)(pktlist->packet+i)->data;
		const UInt16 length = (pktlist->packet+i)->length;
		_handler->HandleMidiData(data, length);
	}
}

void
CoreMidiInterface::midiNotifyProc (const MIDINotification *message, void *refCon)
{
}

void
CoreMidiInterface::midiReadProc (const MIDIPacketList *pktlist, void *readProcRefCon, void *srcConnRefCon)
{
	CoreMidiInterface* self = (CoreMidiInterface *)readProcRefCon;
	self->HandleMidiPacketList(pktlist);
}

#else
MidiInterface* CreateCoreMidiInterface() { return NULL; }
#endif
