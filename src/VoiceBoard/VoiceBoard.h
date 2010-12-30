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
	:	buffer		(new float[bufsize*5])
	,	osc_1		(buffer+bufsize*0)
	,	osc_2		(buffer+bufsize*1)
	,	lfo_osc_1	(buffer+bufsize*2)
	,	filter_env	(buffer+bufsize*3)
	,	amp_env		(buffer+bufsize*4)
	{}
	~VoiceBoardProcessMemory () { delete [] buffer; }

private:
	float * buffer;

public:	
	float *	const osc_1;
	float *	const osc_2;
	float *	const lfo_osc_1;
	float *	const filter_env;
	float *	const amp_env;
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
	VoiceBoard(const VoiceBoardProcessMemory * mem);

	bool	isSilent		();
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

	const VoiceBoardProcessMemory * mem;
	
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
	IIRFilterFirstOrder _vcaFilter;
	float			mAmpModAmount;
	ADSR 			amp_env;
};

#endif
