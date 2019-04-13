/*
 *  CoreAudio.cpp
 *
 *  Copyright (c) 2001-2014 Nick Dowell
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

#include "CoreAudio.h"

#include "../Configuration.h"

#if (__APPLE__)

#include <CoreAudio/CoreAudio.h>
#include <CoreMIDI/MIDIServices.h>
#include <vector>

#define MIDI_BUFFER_SIZE 4096


class CoreAudioOutput : public GenericOutput
{
public:

	CoreAudioOutput() : m_DeviceID(0), m_clientRef(0), m_MIDIBuffer(NULL), m_MIDIBufferWriteIndex(0), m_MIDIBufferReadIndex(0)
	{
		UInt32 size; Boolean writable; OSStatus err;
		
		err = AudioHardwareGetPropertyInfo (kAudioHardwarePropertyDevices, &size, &writable);
		if (kAudioHardwareNoError != err) return;
		
		m_DeviceList = new AudioDeviceID[size / sizeof (AudioDeviceID)];
		
		err = AudioHardwareGetProperty (kAudioHardwarePropertyDevices, &size, m_DeviceList);
		if (kAudioHardwareNoError != err)
		{
			delete[] m_DeviceList;
			m_DeviceList = 0;
		}
	}
	
	~CoreAudioOutput()
	{
		Stop();
        MIDIClientDispose(m_clientRef);
		delete[] m_DeviceList;
	}
	
	virtual int init()
	{
		if (!m_DeviceList) {
            return -1;
        }

        Configuration & config = Configuration::get();
        config.current_audio_driver = "CoreAudio";
        config.current_midi_driver = "CoreMIDI";

        m_currInput = MIDIGetSource(0);
        if (m_currInput) {
            MIDIPortConnectSource(m_inPort, m_currInput, NULL);
        }
		
		return 0;
	}
	
	virtual bool Start()
	{
		if (!m_DeviceID)
		{
			OSStatus status = noErr;
			AudioDeviceID defaultDeviceID = 0;
			UInt32 propertySize = sizeof(defaultDeviceID);
			status = AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice, &propertySize, &defaultDeviceID);
			if (status != kAudioHardwareNoError) return false;
			m_DeviceID = defaultDeviceID;

			AudioDeviceAddIOProc(m_DeviceID, audioDeviceIOProc, (void *)this);
			AudioDeviceStart(m_DeviceID, audioDeviceIOProc);
		}

        m_MIDIBuffer = (uint32_t *)calloc(MIDI_BUFFER_SIZE, 1);
        MIDIClientCreate(CFSTR("amsynth"), NULL, this, &m_clientRef);
        MIDIInputPortCreate(m_clientRef, CFSTR("Input port"), midiReadProc, this, &m_inPort);
        
        for (unsigned long i = 0; i < MIDIGetNumberOfSources(); i++) {
            MIDIPortConnectSource(m_inPort, MIDIGetSource(i), this);
        }

		return 0;
	}
	
	virtual void Stop()
	{
		if (m_DeviceID) {
			AudioDeviceStop(m_DeviceID, audioDeviceIOProc);
			m_DeviceID = 0;
		}
        
        if (m_currInput) {
            MIDIPortDisconnectSource(m_inPort, m_currInput);
            m_currInput = 0;
        }
	}
    
    static OSStatus audioDeviceIOProc(AudioDeviceID           inDevice,
                                      const AudioTimeStamp*   inNow,
                                      const AudioBufferList*  inInputData,
                                      const AudioTimeStamp*   inInputTime,
                                      AudioBufferList*        outOutputData,
                                      const AudioTimeStamp*   inOutputTime,
                                      void*                   inClientData)
    {
        CoreAudioOutput *self = (CoreAudioOutput *)inClientData;

        float *outL = NULL;
        float *outR = NULL;
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
        
        std::vector<amsynth_midi_event_t> midi_events;
        
        unsigned idx = self->m_MIDIBufferReadIndex;
        while (idx != self->m_MIDIBufferWriteIndex) {
            unsigned length = self->m_MIDIBuffer[idx];
            assert(length <= 3);
            amsynth_midi_event_t event = {
                .offset_frames = 0,
                .length = length,
                .buffer = (unsigned char *)(self->m_MIDIBuffer + idx + 1)
            };
            midi_events.push_back(event);
            self->m_MIDIBuffer[idx] = 0;
            idx = (idx + length + 1) % MIDI_BUFFER_SIZE;
        }
        self->m_MIDIBufferReadIndex = idx;

        std::vector<amsynth_midi_cc_t> midi_out;
        amsynth_audio_callback(outL, outR, numSampleFrames, stride, midi_events, midi_out);
		
		return noErr;
    }

    static void midiReadProc(const MIDIPacketList *pktlist, void *readProcRefCon, void *srcConnRefCon)
    {
        CoreAudioOutput *self = (CoreAudioOutput *)srcConnRefCon;
        
        for (UInt32 i = 0; i < pktlist->numPackets; i++) {
            uint8_t *data = (unsigned char *)(pktlist->packet + i)->data;
            const UInt16 length = (pktlist->packet + i)->length;
            if (length > 3) {
                continue;
            }
            unsigned idx = self->m_MIDIBufferWriteIndex;
            self->m_MIDIBuffer[idx] = length;
            memcpy(self->m_MIDIBuffer + idx + 1, data, length);
            self->m_MIDIBufferWriteIndex = (idx + length + 1) % MIDI_BUFFER_SIZE;
        }
    }
	
private:

	AudioDeviceID* m_DeviceList;
	unsigned long  m_DeviceListSize;
	AudioDeviceID  m_DeviceID;
    
    MIDIClientRef   m_clientRef;
	MIDIPortRef     m_inPort;
	MIDIEndpointRef m_currInput;
    
    uint32_t *m_MIDIBuffer;
    unsigned m_MIDIBufferWriteIndex;
    unsigned m_MIDIBufferReadIndex;
};

GenericOutput * CreateCoreAudioOutput() { return new CoreAudioOutput; }

#else

GenericOutput* CreateCoreAudioOutput() { return NULL; }

#endif
