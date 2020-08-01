/*
 *  Oscillator.cpp
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

#include "Oscillator.h"

#include <algorithm>
#include <cassert>
#include <climits>
#include <cmath>
#include <cstdlib>

#define ALIAS_REDUCTION

static inline float ffmodf(float x, float y) {
	return (x - y * (int)(x / y));
}

#define DO_OSC_SYNC(__osc_rads__) \
	if (mSyncEnabled) { \
		mSyncRads = mSyncRads + twopi_rate * mSyncFrequency; \
		if (mSyncRads >= m::twoPi) { \
			mSyncRads -= m::twoPi; \
			__osc_rads__ = 0; \
		} \
	}

void Oscillator::SetWaveform	(Waveform w)			{ waveform = w; }
void Oscillator::reset			()						{ rads = 0.0; }

void
Oscillator::SetSampleRate(int rateIn)
{
	rate = rateIn;
	twopi_rate = m::twoPi / rate;
}

void
Oscillator::setPolarity(float polarity)
{
	assert(polarity == 1.0 || polarity == -1.0);
	mPolarity = polarity;
}

void
Oscillator::ProcessSamples	(float *buffer, int nFrames, float freq_hz, float pw, float sync_freq)
{
	float maxFreq = rate / 2.f;
	mFrequency.configure(mFrequency.getFinalValue(), std::min(freq_hz, maxFreq), nFrames);
	mPulseWidth = pw;
	mSyncFrequency = sync_freq;
	
	switch (waveform) {
	case Waveform::kSine:     doSine      (buffer, nFrames); break;
	case Waveform::kPulse:    doSquare    (buffer, nFrames); break;
	case Waveform::kSaw:      doSaw       (buffer, nFrames); break;
	case Waveform::kNoise:    doNoise     (buffer, nFrames); break;
	case Waveform::kRandom:   doRandom    (buffer, nFrames); break;
	}
}

void
Oscillator::doSine(float *buffer, int nFrames)
{
    for (int i = 0; i < nFrames; i++) {
		DO_OSC_SYNC(rads);
		buffer[i] = sinf(rads += twopi_rate * mFrequency.nextValue());
	}
	rads = ffmodf(rads, m::twoPi);			// overflows are bad!
}

void 
Oscillator::doSquare(float *buffer, int nFrames)
{
	const float radsper = twopi_rate * mFrequency.getFinalValue();
	const float pwscale = radsper < 0.3f ? 1.0f : 1.0f - ((radsper - 0.3f) / 2); assert(pwscale <= 1.0f); // reduces aliasing at high freq
	const float pwrads = m::pi + pwscale * m::pi * std::min(mPulseWidth, 0.9f);

	float lrads = rads;

    for (int i = 0; i < nFrames; i++) {
		DO_OSC_SYNC(lrads);

		float radinc = twopi_rate * mFrequency.nextValue();
		float nrads = lrads + radinc;
		float y = 0.0f;

		//
		// aliasing is reduced by computing accurate values at crossing points (rather than always forcing -1.0 or 1.0.)
		// cpu performance is surprisingly good on x86 (better than saw or sine wave), probably due to its sophisticated branch prediction.
		//
		if (nrads >= m::twoPi) // transition from -1 --> 1
		{
			nrads -= m::twoPi;
			float amt = nrads / radinc; assert(amt <= 1.001f);
			y = (2.0f * amt) - 1.0f;
		}
		else if (nrads <= pwrads)
		{
			y = 1.0f;
		}
		else if (lrads <= pwrads) // transition from 1 --> -1
		{
			float amt = (nrads - pwrads) / radinc; assert(amt <= 1.001f);
			y = 1.0f - (2.0f * amt);
		}
		else
		{
			y = -1.0f;
		}

		buffer[i] = y;
		lrads = nrads; assert(lrads < m::twoPi);
	}
	rads = lrads;
}

static inline float saw(float rads, float shape)
{
    rads = ffmodf(rads, m::twoPi);

    float t = rads / m::twoPi;
    float a = (shape + 1.0f) / 2.0f;

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
		DO_OSC_SYNC(rads);
		buffer[i] = saw(rads += (twopi_rate * mFrequency.nextValue()), mPulseWidth) * mPolarity;
	}
    rads = ffmodf(rads, m::twoPi);

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
    int period = (int) (rate / mFrequency.getFinalValue());
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
