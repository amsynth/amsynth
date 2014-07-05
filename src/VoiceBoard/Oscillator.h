/*
 *  Oscillator.h
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
	enum Waveform { 
		Waveform_Sine,
		Waveform_Pulse,
		Waveform_Saw,
		Waveform_Noise,
		Waveform_Random
	};

	Oscillator	();

	void	SetSampleRate	(int rateIn);
	
	void	ProcessSamples		(float*, int, float freq_hz, float pw, float sync_freq = 0);

	void	SetWaveform		(Waveform);
	Waveform GetWaveform() { return waveform; }

	void reset();
	
	void	setSyncEnabled(bool sync) { mSyncEnabled = sync; }
	void	setPolarity (float polarity); // +1 or -1

private:
    float rads, twopi_rate, random;
	double a0, a1, b1, d; // for the low-pass filter
    int rate, random_count;

	Waveform waveform;
	Lerper	mFrequency;
	float	mPulseWidth;
	float	mPolarity;
	
	float	mSyncFrequency;
	bool	mSyncEnabled;
	double	mSyncRads;
	
    void doSine(float*, int nFrames);
    void doSquare(float*, int nFrames);
    void doSaw(float*, int nFrames);
    void doNoise(float*, int nFrames);
	void doRandom(float*, int nFrames);
};

#endif				/// _OSCILLATOR_H
