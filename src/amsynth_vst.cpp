/*
 *  amsynth_vst.cpp
 *
 *  Copyright (c) 2008-2014 Nick Dowell
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

/* To build the amsynth VST plugin;
 * 1) download the VST 2.4 SDK and unzip into the src/ directory
 * 2) run: patch vstsdk2.4/pluginterfaces/vst2.x/aeffect.h aeffect.h.patch
 * 3) run: make -f Makefile.linux.vst
 * this should output amsynth.vst.so
 */


#include "PresetController.h"
#include "VoiceAllocationUnit.h"

#include "public.sdk/source/vst2.x/audioeffectx.h"


class AmsynthVST : public AudioEffectX
{
public:
	
	AmsynthVST(audioMasterCallback callback) : AudioEffectX(callback, PresetController::kNumPresets, kAmsynthParameterCount)
	{
		setUniqueID('amsy');
		setNumInputs(0);
		setNumOutputs(2);
		canProcessReplacing(true);
		isSynth(true);
		
		Preset & preset = mPresetController.getCurrentPreset();
		for (unsigned i=0; i<preset.ParameterCount(); i++) {
			Parameter & param = preset.getParameter(i);
			mVoiceAllocationUnit.UpdateParameter( param.GetId(), param.getControlValue() );
		}
	}
	
	virtual void setParameter(VstInt32 index, float value)
	{
		Parameter & param = mPresetController.getCurrentPreset().getParameter(index);
		param.SetNormalisedValue(value);
		mVoiceAllocationUnit.UpdateParameter( param.GetId(), param.getControlValue() );
	}
	
	virtual float getParameter(VstInt32 index)
	{
		Parameter & param = mPresetController.getCurrentPreset().getParameter(index);
		return param.GetNormalisedValue();
	}
	
	virtual void getParameterLabel(VstInt32 index, char * text)
	{
		strncpy( text, mPresetController.getCurrentPreset().getParameter(index).getLabel().c_str(), 32 );
	}
	
	virtual void getParameterDisplay(VstInt32 index, char * text)
	{
		strncpy( text, mPresetController.getCurrentPreset().getParameter(index).GetStringValue().c_str(), 32 );
	}
	
	virtual void getParameterName(VstInt32 index, char * text)
	{
		strncpy( text, mPresetController.getCurrentPreset().getParameter(index).getName().c_str(), 32 );
	}
	
	virtual void setProgram(VstInt32 program)
	{
		mPresetController.selectPreset(program);
	}
	
	virtual void setProgramName(char* name)
	{
		mPresetController.getCurrentPreset().setName( std::string(name) );
	}
	
	virtual void getProgramName(char* name)
	{
		strncpy( name, mPresetController.getCurrentPreset().getName().c_str(), kVstMaxProgNameLen );
	}
	
	virtual VstInt32 processEvents(VstEvents * events)
	{
		for (VstInt32 i=0; i<events->numEvents; i++)
			if (events->events[i]->type == kVstMidiType)
				processEvent( *reinterpret_cast<VstMidiEvent*>(events->events[i]) );
		return 1;
	}

	void processEvent(const VstMidiEvent & event)
	{
		const char * midiData = event.midiData;
		const VstInt32 status = midiData[0] & 0xf0;
		
		switch (status)
		{
		case 0x80:
		case 0x90:
			static const float scale = 1.f / 127.f;
			const VstInt32 note = midiData[1] & 0x7f;
			const VstInt32 velocity = (status == 0x90) ? midiData[2] & 0x7f : 0;
			
			if (velocity)
				mVoiceAllocationUnit.HandleMidiNoteOn( note, velocity * scale );
			else
				mVoiceAllocationUnit.HandleMidiNoteOff( note, velocity * scale );
			
			break;
		}
	}
	
	virtual void processReplacing( float ** inputs, float ** outputs, VstInt32 numSampleFrames )
	{
		mVoiceAllocationUnit.Process(outputs[0], outputs[1], numSampleFrames, 1);
	}
	
protected:
	
	VoiceAllocationUnit	mVoiceAllocationUnit;
	PresetController	mPresetController;
};


extern "C" AEffect * VSTPluginMain(audioMasterCallback audioMaster)
{
	AudioEffectX *plugin = new AmsynthVST(audioMaster);
	if (!plugin) {
		return NULL;
	}
	return plugin->getAeffect();
}

// this is required because GCC throws an error if we declare a non-standard function named 'main'
extern "C" __attribute__ ((visibility("default"))) AEffect * main_plugin(audioMasterCallback audioMaster) asm ("main");

extern "C" __attribute__ ((visibility("default"))) AEffect * main_plugin(audioMasterCallback audioMaster)
{
	return VSTPluginMain (audioMaster);
}
