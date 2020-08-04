/*
 *  Oscillator.h
 *
 *  Copyright (c) 2001-2020 Nick Dowell
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

#ifndef _OSCILLATOR_H
#define _OSCILLATOR_H

#include "Synth--.h"

/**
 * @brief An Audio Oscillator unit.
 * 
 * Provides several different output waveforms (sine, saw, square, noise, 
 * random).
 */
class Oscillator
{
public:
	enum class Waveform { 
		kSine,
		kPulse,
		kSaw,
		kNoise,
		kRandom
	};

	void	SetSampleRate	(int rateIn);
	
	void	ProcessSamples		(float*, int, float freq_hz, float pw, float sync_freq = 0);

	void	SetWaveform		(Waveform);
	Waveform GetWaveform() { return waveform; }

	void reset();
	
	void	setSyncEnabled(bool sync) { mSyncEnabled = sync; }
	void	setPolarity (float polarity); // +1 or -1

private:
    float rads = 0;
	float twopi_rate = 0;
	float random = 0;
	double d = 0; // for the low-pass filter
    int rate = 44100;
	int random_count = 0;

	Waveform waveform = Waveform::kSine;
	Lerper	mFrequency;
	float	mPulseWidth = 0;
	float	mPolarity = 1;
	
	float	mSyncFrequency = 0;
	bool	mSyncEnabled = false;
	double	mSyncRads = 0;
	
    void doSine(float*, int nFrames);
    void doSquare(float*, int nFrames);
    void doSaw(float*, int nFrames);
    void doNoise(float*, int nFrames);
	void doRandom(float*, int nFrames);
};

#endif				/// _OSCILLATOR_H
