/*
 *  Oscillator.cc
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

#include "Oscillator.h"

#include "Synth--.h"

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <limits.h>

#define ALIAS_REDUCTION

static inline float ffmodf(float x, float y) {
	return (x - y * (int)(x / y));
}

Oscillator::Oscillator()
:	rads (0.0)
,	random (0)
,	waveform (Waveform_Sine)
,	rate (44100)
,	random_count (0)
,	mPolarity(1.0f)
,	reset_offset (4096)
,	reset_period (4096)
,	sync (NULL)
{
}

void Oscillator::SetWaveform	(Waveform w)			{ waveform = w; }
void Oscillator::reset			()						{ rads = 0.0; }
void Oscillator::reset			(int offset, int period){ reset_offset = offset; reset_period = period; }

void Oscillator::SetSync		(Oscillator* o)
{
	if (sync) sync->reset (4096, 4096);
	sync = o;
	reset_period = 4096;
	reset_offset = 4096;
}

void
Oscillator::SetSampleRate(int rateIn)
{
	rate = rateIn;
	twopi_rate = (float) TWO_PI / rate;
}

void
Oscillator::setPolarity(float polarity)
{
	assert(polarity == 1.0 || polarity == -1.0);
	mPolarity = polarity;
}

void
Oscillator::ProcessSamples	(float *buffer, int numSamples, float freq_hz, float pw)
{
	mFrequency.configure(mFrequency.getFinalValue(), freq_hz, numSamples);
	mPulseWidth = pw;
	
	sync_c = 0;
	sync_offset = 65;
		
	reset_cd = reset_offset;
	
	switch (waveform)
	{
	case Waveform_Sine:		doSine		(buffer, numSamples);	break;
	case Waveform_Pulse:	doSquare	(buffer, numSamples);	break;
	case Waveform_Saw:		doSaw		(buffer, numSamples);	break;
	case Waveform_Noise:	doNoise		(buffer, numSamples);	break;
	case Waveform_Random:	doRandom	(buffer, numSamples);	break;
	default: break;
	}
	
	if (sync) sync->reset (sync_offset, (int)(rate/freq_hz));
}

void 
Oscillator::doSine(float *buffer, int nFrames)
{
    for (int i = 0; i < nFrames; i++) {
		buffer[i] = sinf(rads += twopi_rate * mFrequency.nextValue());
		//-- sync to other oscillator --
		if (reset_cd-- == 0){
			rads = 0.0;					// reset the oscillator
			reset_cd = reset_period-1;	// start counting down again
		}
		if ( sync_offset > nFrames)	// then we havent already found the offset
			if( rads > TWO_PI )			// then weve completed a circle
				sync_offset = i;		// remember the offset
	}
	rads = ffmodf((float)rads, (float)TWO_PI);			// overflows are bad!
}

void 
Oscillator::doSquare(float *buffer, int nFrames)
{
	const float radsper = twopi_rate * mFrequency.getFinalValue();
	const float pwscale = radsper < 0.4f ? 1.0f : 1.0f - ((radsper - 0.4f) / 2); // reduces aliasing at high freq
	const float pwrads = PI + pwscale * PI * MIN(mPulseWidth, 0.9f);

    for (int i = 0; i < nFrames; i++) {
		float radinc = twopi_rate * mFrequency.nextValue();
		float nrads = rads + radinc;
		float y = 0.0f;

		//
		// aliasing is reduced by computing accurate values at crossing points (rather than always forcing -1.0 or 1.0.)
		// cpu performance is surprisingly good on x86 (better than saw or sine wave), probably due to its sophisticated branch prediction.
		//
		if (nrads >= TWO_PI) // transition from -1 --> 1
		{
			nrads -= TWO_PI;
			float amt = nrads / radinc; assert(amt <= 1.0f);
			y = -1.0f + (2.0f * amt);
		}
		else if (nrads < pwrads)
		{
			y = 1.0f;
		}
		else if (rads <= pwrads) // transition from 1 --> -1
		{
			float amt = (nrads - pwrads) / radinc; assert(amt <= 1.0f);
			y = 1.0f - (2.0f * amt);
		}
		else
		{
			y = -1.0f;
		}

		buffer[i] = y;
		rads = nrads;

		//-- sync to other oscillator -- ##**.. THIS ALIASES TERRIBLY ..**##
		if (reset_cd-- == 0){
			rads = 0.0;					// reset the oscillator
			reset_cd = reset_period-1;	// start counting down again
		}
		if ( sync_offset > nFrames)	// then we havent already found the offset
			if( rads > TWO_PI )			// then weve completed a circle
				sync_offset = i;		// remember the offset
	}
}

float
Oscillator::saw(float rads)
{
    rads = ffmodf((float)rads, (float)TWO_PI);

    float t = rads / (float)TWO_PI;
    float a = (mPulseWidth + 1.0f) / 2.0f;

    if (t < a / 2)
		return 2 * t / a;

    if (t > (1 - (a / 2)))
		return (2 * t - 2) / a;

	return (1 - 2 * t) / (1 - a);
}

void 
Oscillator::doSaw(float *buffer, int nFrames)
{
#ifdef ALIAS_REDUCTION
	// Clamp the maximum slope to reduce amount of aliasing in high octaves.
	// This is not proper anti-aliasing ;-)
	const float requestedPW = mPulseWidth;
	const float kAliasReductionAmount = 2.0f;
	const float f = requestedPW - (kAliasReductionAmount * mFrequency.getFinalValue() / (float)rate);
	if (mPulseWidth > f)
		mPulseWidth = f;
#endif

    for (int i = 0; i < nFrames; i++) {
		buffer[i] = saw(rads += (twopi_rate * mFrequency.nextValue())) * mPolarity;
		//-- sync to other oscillator --
		if (reset_cd-- == 0){
			rads = 0.0;					// reset the oscillator
			reset_cd = reset_period-1;	// start counting down again
		}
		if ( sync_offset > nFrames)	// then we havent already found the offset
			if( rads > TWO_PI )			// then weve completed a circle
				sync_offset = i;		// remember the offset
	}
    rads = ffmodf((float)rads, (float)TWO_PI);

#ifdef ALIAS_REDUCTION
	mPulseWidth = requestedPW;
#endif
}

static const float kTwoOverUlongMax = 2.0f / (float)ULONG_MAX;

static inline float randf()
{
	// Calculate pseudo-random 32 bit number based on linear congruential method.
	// http://www.musicdsp.org/showone.php?id=59
	static unsigned long random = 22222;
	random = (random * 196314165) + 907633515;
	return (float)random * kTwoOverUlongMax - 1.0f;
}

void 
Oscillator::doRandom(float *buffer, int nFrames)
{
    register int period = (int) (rate / mFrequency.getFinalValue());
    for (int i = 0; i < nFrames; i++) {
	if (random_count > period) {
	    random_count = 0;
		random = randf();
	}
	random_count++;
	buffer[i] = random;
    }
}

void 
Oscillator::doNoise(float *buffer, int nFrames)
{
    for (int i = 0; i < nFrames; i++)
		buffer[i] = randf();
}
