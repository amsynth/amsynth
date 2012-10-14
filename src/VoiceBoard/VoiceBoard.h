/* amSynth
 * (c) 2001-2004 Nick Dowell
 */

#ifndef _VOICEBOARD_H
#define _VOICEBOARD_H

#include "../controls.h"
#include "ADSR.h"
#include "Oscillator.h"
#include "LowPassFilter.h"

/**
 * the VoiceBoard is what makes the nice noises... ;-)
 *
 * one VoiceBoard is a 'voice' in its own right, and play only one note at a 
 * time. the VoiceAllocationUnit decides which voices do what etc...
 **/

class VoiceBoard
{
public:

	enum {
		kMaxProcessBufferSize = 64,
	};

	VoiceBoard();

	bool	isSilent		();
	void	triggerOn		();
	void	triggerOff		();
	void	setVelocity		(float velocity);
	
	void	setFrequency	(float targetFrequency, float glissandoTime = 0.0f);
	float	getFrequency	() { return mFrequency.getValue(); }
	
	void	SetPitchBend	(float);
	void	reset			();

	void	UpdateParameter		(Param, float);

	void	ProcessSamplesMix	(float *buffer, int numSamples, float vol);

	void	SetSampleRate		(int);

private:

	Lerper			mFrequency;
	float			mSampleRate;
	float			mKeyVelocity;
	float			mPitchBend;
	
	// modulation section
	Oscillator 		lfo1;
	float			mLFO1Freq;
	
	// oscillator section
	Oscillator 		osc1, osc2;
	float			mFreqModAmount;
	float			mOsc1PulseWidth;
	float			mOsc2PulseWidth;
	float			mOsc1Vol;
	float			mOsc2Vol;
	float			mRingModAmt;
	float			mOsc2Octave;
	float			mOsc2Detune;
	
	// filter section
	float			mFilterEnvAmt,
				mFilterModAmt,
				mFilterCutoff,
				mFilterRes;
	LowPassFilter 		filter;
	ADSR 			filter_env;
	
	// amp section
	IIRFilterFirstOrder _vcaFilter;
	float			mAmpModAmount;
	ADSR 			amp_env;
};

#endif
