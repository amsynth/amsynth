/*
 *  VoiceBoard.h
 *
 *  Copyright (c) 2001-2012 Nick Dowell
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

	enum {
		kMaxProcessBufferSize = 64,
	};

	VoiceBoard();

	bool	isSilent		();
	void	triggerOn		();
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

	Lerper			mFrequency;
	bool			mFrequencyDirty;
	float			mFrequencyStart;
	float			mFrequencyTarget;
	float			mFrequencyTime;

	float			mSampleRate;
	float			mKeyVelocity;
	float			mPitchBend;
	
	// modulation section
	Oscillator 		lfo1;
	float			mLFO1Freq;
	float			mLFOPulseWidth;
	
	// oscillator section
	Oscillator 		osc1, osc2;
	float			mFreqModAmount;
	int			mFreqModDestination;
	float			mOsc1PulseWidth;
	float			mOsc2PulseWidth;
	float			mOsc1Vol;
	float			mOsc2Vol;
	float			mRingModAmt;
	float			mOsc2Octave;
	float			mOsc2Detune;
	float			mOsc2Pitch;
	bool			mOsc2Sync;
	
	// filter section
	float			mFilterEnvAmt,
				mFilterModAmt,
				mFilterCutoff,
				mFilterRes;
	float			mFilterKbdTrack;
	float			mFilterVelSens;
	SynthFilter 	filter;
	SynthFilter::FilterType mFilterType;
	SynthFilter::FilterSlope mFilterSlope;
	ADSR 			filter_env;
	
	// amp section
	IIRFilterFirstOrder _vcaFilter;
	float			mAmpModAmount;
	float			mAmpVelSens;
	ADSR 			amp_env;

	struct {
		float osc_1[kMaxProcessBufferSize];
		float osc_2[kMaxProcessBufferSize];
		float lfo_osc_1[kMaxProcessBufferSize];
		float filter_env[kMaxProcessBufferSize];
		float amp_env[kMaxProcessBufferSize];
	} mProcessBuffers;
};

#endif
