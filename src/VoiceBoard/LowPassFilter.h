/*
 *  LowPassFilter.h
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

#ifndef _LOWPASSFILTER_H
#define _LOWPASSFILTER_H

class SynthFilter
{
public:

	enum class Type {
		kLowPass,
		kHighPass,
		kBandPass,
		kBandStop,
		kBypass
	};

	enum class Slope {
		k12,
		k24,
	};

	void SetSampleRate(int rateIn) { rate = (float)rateIn; nyquist = rate / 2.0f; }

	void reset();

	void ProcessSamples(float *, int, float cutoff, float res, Type type, Slope slope);

private:

	float rate = 44100;
	float nyquist = 22050.0;
	double d1 = 0;
	double d2 = 0;
	double d3 = 0;
	double d4 = 0;
};

#endif
