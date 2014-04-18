/*
 *  Synth--.h
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

#ifndef _SYNTH_MM_H
#define _SYNTH_MM_H

#include <cmath>

#ifndef M_E
#define M_E		2.7182818284590452354
#endif

#ifndef M_PI_2
#define M_PI_2	1.57079632679489661923
#endif

#define TWO_PI 6.28318530717958647692
#define PI     3.14159265358979323846

#ifndef MIN
#define MIN(a,b) (((a) < (b)) ? (a) : (b))
#endif

class Lerper
{

public:

	Lerper(): _start(0), _final(0), _inc(0), _steps(0), _i(0) {}
	
	void configure(float startValue, float finalValue, unsigned int numSteps)
	{
		_start = startValue;
		_final = finalValue;
		_steps = numSteps;
		if (0 < _steps) {
			_inc = (_final - _start) / (float)_steps;
		} else {
			_inc = 0.0f;
			_start = finalValue;
		}
		_i = 0;
	}

	inline float getValue() const
	{
		return _start + _i * (float)_inc;
	}
	
	inline float nextValue()
	{
		float y = getValue();
		_i = MIN(_i + 1, _steps);
		return y;
	}

	inline float getFinalValue() const
	{
		return _final;
	}
	
private:

	float _start, _final, _inc;
	unsigned int _steps, _i;

};

struct IIRFilterFirstOrder
{
	enum Mode
	{
		LowPass,
		HighPass,
	};
	
	IIRFilterFirstOrder()
		:	_a0(0.0f)
		,	_a1(0.0f)
		,	_b1(0.0f)
		,	_z(0.0f)
	{}
	
	void setCoefficients(float sampleRate, float cutoffFreq, Mode mode)
	{
		float fc, x;
		fc = cutoffFreq / sampleRate;
		fc = MIN(fc, 0.5f);
		x = powf(M_E, -M_PI_2 * fc);
		if (LowPass == mode) {
			_a0 = 1.0f - x;
			_a1 = 0.0f;
			_b1 = x;
		} else {
			_a0 =  (1 + x) / 2.0f;
			_a1 = -(1 + x) / 2.0f;
			_b1 = x;
		}
	}
	
	inline float processSample(float x)
	{
		float y = (x * _a0) + _z;
		_z = (x * _a1) + (y * _b1);
		return y;
	}
	
	void processBuffer(float *samples, unsigned numSamples)
	{
		for (unsigned i=0; i<numSamples; i++)
			samples[i] = processSample(samples[i]);
	}
	
	float _a0, _a1, _b1, _z;
};

#endif
