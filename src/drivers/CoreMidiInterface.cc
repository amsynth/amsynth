/*
 *  CoreMidiInterface.cc
 *  amsynth
 */

#include "CoreMidiInterface.h"

#if (__APPLE__)

#pragma mark - CoreAudio

#include <vector>
#include <CoreAudio/CoreAudio.h>

#include "../VoiceAllocationUnit.h"

OSStatus audioDeviceIOProc (   AudioDeviceID           inDevice,
							   const AudioTimeStamp*   inNow,
							   const AudioBufferList*  inInputData,
							   const AudioTimeStamp*   inInputTime,
							   AudioBufferList*        outOutputData,
							   const AudioTimeStamp*   inOutputTime,
							   void*                   inClientData)
{
	float* outL;
	float* outR;
	unsigned numSampleFrames = 0;
	unsigned stride = 1;
	
	if (1 < outOutputData->mBuffers[0].mNumberChannels)
	{
		numSampleFrames = outOutputData->mBuffers[0].mDataByteSize / (outOutputData->mBuffers[0].mNumberChannels * sizeof(float));
		stride = outOutputData->mBuffers[0].mNumberChannels;
		outL = (float*)outOutputData->mBuffers[0].mData;
		outR = outL + 1;
	}
	else if (1 < outOutputData->mNumberBuffers)
	{
		numSampleFrames = outOutputData->mBuffers[0].mDataByteSize / (outOutputData->mBuffers[0].mNumberChannels * sizeof(float));
		outL = (float*)outOutputData->mBuffers[0].mData;
		outR = (float*)outOutputData->mBuffers[1].mData;
	}
	else
	{
		return kAudioDeviceUnsupportedFormatError;
	}
	
	VoiceAllocationUnit** vau = (VoiceAllocationUnit **)inClientData;
	if (*vau) (*vau)->Process(outL, outR, numSampleFrames, stride);
	
	return noErr;
}


class CoreAudioOutput : public GenericOutput
{
public:
	CoreAudioOutput() : m_DeviceID(0)
	{
		UInt32 size; Boolean writable; OSStatus err;
		
		err = AudioHardwareGetPropertyInfo (kAudioHardwarePropertyDevices, &size, &writable);
		if (kAudioHardwareNoError != err) return;
		
		int m_DeviceListSize = size / sizeof (AudioDeviceID);
		m_DeviceList = new AudioDeviceID[m_DeviceListSize];
		
		err = AudioHardwareGetProperty (kAudioHardwarePropertyDevices, &size, m_DeviceList);
		if (kAudioHardwareNoError != err)
		{
			delete[] m_DeviceList;
			m_DeviceList = 0;
			m_DeviceListSize = 0;
		}
	}
	
	~CoreAudioOutput()
	{
		Stop();
		delete[] m_DeviceList;
		m_DeviceListSize = 0;
	}
	
	virtual int init(Config&)
	{
		return (m_DeviceList && 0 < m_DeviceListSize) ? 0 : -1;
	}
	
	virtual bool Start()
	{
		if (!m_DeviceID)
		{
			m_DeviceID = m_DeviceList[0];
			AudioDeviceAddIOProc(m_DeviceID, audioDeviceIOProc, &mInput);
			AudioDeviceStart(m_DeviceID, audioDeviceIOProc);
		}
		return 0;
	}
	
	virtual void Stop()
	{
		if (m_DeviceID) {
			AudioDeviceStop(m_DeviceID, audioDeviceIOProc);
			m_DeviceID = 0;
		}
	}
	
private: ;
	AudioDeviceID* m_DeviceList;
	unsigned long  m_DeviceListSize;
	AudioDeviceID  m_DeviceID;
};

GenericOutput* CreateCoreAudioOutput() { return new CoreAudioOutput; }


#pragma mark - CoreMIDI

#include <CoreMIDI/MIDIServices.h>

class CoreMidiInterface : public MidiInterface
{
public:
	
	CoreMidiInterface();
	~CoreMidiInterface();
	
	virtual int open(Config&);
	virtual void close();
	
protected:

	virtual void ThreadAction() {};
	
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

#pragma mark - CoreAudio



#else
GenericOutput* CreateCoreAudioOutput() { return NULL; }
MidiInterface* CreateCoreMidiInterface() { return NULL; }
#endif
