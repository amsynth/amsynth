/*
 *  amsynthVst.h
 *  amsynth
 *
 *  Created by Nicolas Dowell on 15/03/2008.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include <vector>

#include "PresetController.h"
#include "VoiceAllocationUnit.h"
#include "public.sdk/source/vst2.x/audioeffectx.h"

class AMSynthVst : public AudioEffectX
{
public:
	AMSynthVst( audioMasterCallback callback )
	:	AudioEffectX( callback, 0, kControls_End )
	{
		setUniqueID('amsy');
		setNumInputs(0);
		setNumOutputs(2);
		canProcessReplacing(true);
		isSynth(true);
		
		Preset & preset = mPresetController.getCurrentPreset();
		for (int i=0; i<preset.ParameterCount(); i++) {
			Parameter & param = preset.getParameter(i);
			mVoiceAllocationUnit.UpdateParameter( param.GetId(), param.getControlValue() );
		}
	}
	
	virtual void setParameter( VstInt32 index, float value )
	{
		Parameter & param = mPresetController.getCurrentPreset().getParameter(index);
		param.SetNormalisedValue(value);
		mVoiceAllocationUnit.UpdateParameter( param.GetId(), param.getControlValue() );
	}
	virtual float getParameter( VstInt32 index )
	{
		Parameter & param = mPresetController.getCurrentPreset().getParameter(index);
		return param.GetNormalisedValue();
	}
	virtual void getParameterLabel( VstInt32 index, char * text )
	{
		strncpy( text, mPresetController.getCurrentPreset().getParameter(index).getLabel().c_str(), 32 );
	}
	virtual void getParameterDisplay( VstInt32 index, char * text )
	{
		strncpy( text, mPresetController.getCurrentPreset().getParameter(index).GetStringValue().c_str(), 32 );
	}
	virtual void getParameterName( VstInt32 index, char * text )
	{
		strncpy( text, mPresetController.getCurrentPreset().getParameter(index).getName().c_str(), 32 );
	}
	
	virtual VstInt32 processEvents( VstEvents * events )
	{
		for (VstInt32 i=0; i<events->numEvents; i++)
			processEvent( *reinterpret_cast<VstMidiEvent*>(events->events[i]) );
		return 1;
	}

	void processEvent( const VstMidiEvent & event )
	{
		if (event.type == kVstMidiType)
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
	}
	
	virtual void processReplacing( float ** inputs, float ** outputs, VstInt32 numSampleFrames )
	{
		mVoiceAllocationUnit.Process(outputs[0], outputs[1], numSampleFrames, 1);
	}
	
protected:

	VoiceAllocationUnit	mVoiceAllocationUnit;
	PresetController	mPresetController;
	
};
