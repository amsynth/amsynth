/*
 *  amsynthVst.h
 *  amsynth
 *
 *  Created by Nicolas Dowell on 15/03/2008.
 *  Copyright 2008 __MyCompanyName__. All rights reserved.
 *
 */

#include "PresetController.h"
#include "VoiceAllocationUnit.h"
#include "audioeffectx.h"

#ifdef WIN32
#include <windows.h>
#define PATH_MAX MAX_PATH
#endif

class AMSynthVst : public AudioEffectX
{
public:
	AMSynthVst( audioMasterCallback callback )
	:	AudioEffectX( callback, PresetController::kNumPresets, kControls_End )
	{
		setUniqueID('amsy');
		setNumInputs(0);
		setNumOutputs(2);
		canProcessReplacing(true);
		isSynth(true);
		
		loadPresets();
		
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
	
	virtual void setProgram (VstInt32 program) { mPresetController.selectPreset(program); }
	
	virtual void setProgramName (char* name)
	{
		mPresetController.getCurrentPreset().setName( std::string(name) );
	}
	virtual void getProgramName (char* name)
	{
		strncpy( name, mPresetController.getCurrentPreset().getName().c_str(), kVstMaxProgNameLen );
	}
	
	virtual VstInt32 processEvents( VstEvents * events )
	{
		for (VstInt32 i=0; i<events->numEvents; i++)
			if (events->events[i]->type == kVstMidiType)
				processEvent( *reinterpret_cast<VstMidiEvent*>(events->events[i]) );
		return 1;
	}

	void processEvent( const VstMidiEvent & event )
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
	
	void loadPresets()
	{
		char filename[PATH_MAX] = "";
		if (getPresetsFilename(filename, PATH_MAX))
			mPresetController.loadPresets(filename);
	}

	bool getPresetsFilename(char * filename, size_t maxLen);
	
	VoiceAllocationUnit	mVoiceAllocationUnit;
	PresetController	mPresetController;
	
};
