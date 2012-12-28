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
	
	void	ProcessSamples		(float*, int, float freq_hz, float pw);
	void	SetWaveform		(Waveform);

	void reset();
	/*
	 * reset the oscillator, initially at sample indicated by offset, and then 
	 * every period samples. used for oscillator sync. 
	 * NB. period >= delta
	 */
    void reset( int offset, int period );

	void	SetSync		(Oscillator*);

	void	setPolarity (float polarity); // +1 or -1

private:
    float rads, twopi_rate, random, freq;
	double a0, a1, b1, d; // for the low-pass filter
    int waveform, rate, random_count;

	float	mPulseWidth;
	float	mPolarity;
	
	// oscillator sync stuff
	int reset_offset, reset_cd, sync_c, sync_offset, sync_period, reset_period;
	Oscillator*	sync;
	
    inline void doSine(float*, int nFrames);
    inline void doSquare(float*, int nFrames);
    inline void doSaw(float*, int nFrames);
    inline void doNoise(float*, int nFrames);
	inline void doRandom(float*, int nFrames);
	inline float sqr(float foo);
	inline float saw(float foo);
};


#endif				/// _OSCILLATOR_H
