/* amSynth
 * (c) 2001-2004 Nick Dowell
 */

#ifndef _VOICEBOARD_H
#define _VOICEBOARD_H

#include "../controls.h"
#include "ADSR.h"
#include "Oscillator.h"
#include "LowPassFilter.h"

class VoiceBoardProcessMemory
{
public:
	VoiceBoardProcessMemory	(int bufsize)
	{
		osc_1 = new float[bufsize];
		osc_2 = new float[bufsize];
		lfo_osc_1 = new float[bufsize];
		filter_env = new float[bufsize];
		amp_env = new float[bufsize];
	}
	VoiceBoardProcessMemory	()
	{
		delete[] osc_1;
		delete[] osc_2;
		delete[] lfo_osc_1;
		delete[] filter_env;
		delete[] amp_env;
	}
		
	float*	osc_1;
	float*	osc_2;
	float*	lfo_osc_1;
	float*	filter_env;
	float*	amp_env;
};
	

/**
 * the VoiceBoard is what makes the nice noises... ;-)
 *
 * one VoiceBoard is a 'voice' in its own right, and play only one note at a 
 * time. the VoiceAllocationUnit decides which voices do what etc...
 **/

class VoiceBoard
{
public:
	VoiceBoard(VoiceBoardProcessMemory *mem);

	int	getState		();
	void	triggerOn		();
	void	triggerOff		();
	void	setVelocity		(float velocity);
	void	setFrequency		(float frequency);
	void	SetPitchBend		(float);
	void	reset			();

	void	UpdateParameter		(Param, float);

	void	ProcessSamplesMix	(float *buffer, int numSamples, float vol);

	void	SetSampleRate		(int);

private:

	VoiceBoardProcessMemory	*mem;
	
	float			mKeyVelocity;
	float			mKeyPitch;
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
	float			mAmpModAmount;
	ADSR 			amp_env;
};

#endif
