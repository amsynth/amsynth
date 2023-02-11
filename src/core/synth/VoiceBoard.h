/*
 *  VoiceBoard.h
 *
 *  Copyright (c) 2001-2021 Nick Dowell
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

#ifndef _VOICEBOARD_H
#define _VOICEBOARD_H

#include "../controls.h"
#include "ADSR.h"
#include "Oscillator.h"
#include "LowPassFilter.h"
#include "Synth--.h"

/**
 * the VoiceBoard is what makes the nice noises... ;-)
 *
 * one VoiceBoard is a 'voice' in its own right, and play only one note at a 
 * time. the VoiceAllocationUnit decides which voices do what etc...
 **/

class VoiceBoard
{
public:

	static constexpr int kMaxProcessBufferSize = 64;

	bool	isSilent		();
	void	triggerOn		(bool reset);
	void	triggerOff		();
	void	setVelocity		(float velocity);
	
	void	setFrequency	(float startFrequency, float targetFrequency, float time = 0.0f);
	float	getFrequency	() { return mFrequency.getValue(); }
	
	void	SetPitchBend	(float);
	void	reset			();

	void	UpdateParameter		(Param, float);

	void	ProcessSamplesMix	(float *buffer, int numSamples, float vol);

	void	SetSampleRate		(int);

private:

	ParamSmoother	mVolume{0.f};

	Lerper			mFrequency;
	bool			mFrequencyDirty = false;
	float			mFrequencyStart = 0;
	float			mFrequencyTarget = 0;
	float			mFrequencyTime = 0;

	float			mSampleRate = 44100;
	float			mKeyVelocity = 1;
	float			mPitchBend = 1;
	
	// modulation section
	Oscillator 		lfo1;
	float			mLFO1Freq = 0;
	float			mLFOPulseWidth = 0;
	
	// oscillator section
	Oscillator 		osc1, osc2;
	float			mFreqModAmount = 0;
	int				mFreqModDestination = 0;
	float			mOsc1PulseWidth = 0;
	float			mOsc2PulseWidth = 0;
	SmoothedParam	mOscMix{0.f};
	SmoothedParam	mRingModAmt{0.f};
	float			mOsc2Octave = 1;
	float			mOsc2Detune = 1;
	float			mOsc2Pitch = 0;
	bool			mOsc2Sync = false;
	
	// filter section
	float			mFilterEnvAmt = 0;
	float			mFilterModAmt = 0;
	float			mFilterCutoff = 16;
	float			mFilterRes = 0;
	float			mFilterKbdTrack = 0;
	float			mFilterVelSens = 0;
	SynthFilter 	filter;
	SynthFilter::Type mFilterType;
	SynthFilter::Slope mFilterSlope;
	ADSR 			mFilterADSR;
	
	// amp section
	IIRFilterFirstOrder _vcaFilter;
	SmoothedParam	mAmpModAmount{-1.f};
	SmoothedParam	mAmpVelSens{1.f};
	ADSR 			mAmpADSR;

	struct {
		float osc_1[kMaxProcessBufferSize];
		float osc_2[kMaxProcessBufferSize];
		float lfo_osc_1[kMaxProcessBufferSize];
		float filter_env[kMaxProcessBufferSize];
		float amp_env[kMaxProcessBufferSize];
	} mProcessBuffers;
};

#endif
