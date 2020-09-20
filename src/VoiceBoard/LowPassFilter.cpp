/*
 *  LowPassFilter.cpp
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

#include "LowPassFilter.h"
#include "Synth--.h"

#include <algorithm>
#include <cassert>
#include <math.h>

void
SynthFilter::reset()
{
	d1 = d2 = d3 = d4 = 0;
}

void
SynthFilter::ProcessSamples(float *buffer, int numSamples, float cutoff, float res, Type type, Slope slope)
{
	if (type == Type::kBypass) {
		return;
	}
	
	cutoff = std::min(cutoff, nyquist * 0.99f); // filter is unstable at PI
	cutoff = std::max(cutoff, 10.0f);

	const double w = (cutoff / rate); // cutoff freq [ 0 <= w <= 0.5 ]
	const double r = std::max(0.001, 2.0 * (1.0 - res)); // r is 1/Q (sqrt(2) for a butterworth response)

	const double k = tan(w * m::pi);
	const double k2 = k * k;
	const double rk = r * k;
	const double bh = 1.0 + rk + k2;

	double a0, a1, a2, b1, b2;

	switch (type) {
		case Type::kLowPass:
			//
			// Bilinear transformation of H(s) = 1 / (s^2 + s/Q + 1)
			// See "Digital Audio Signal Processing" by Udo Zölzer
			//
			a0 = k2 / bh;
			a1 = a0 * 2.0;
			a2 = a0;
			b1 = (2.0 * (k2 - 1.0)) / bh;
			b2 = (1.0 - rk + k2) / bh;
			break;

		case Type::kHighPass:
			//
			// Bilinear transformation of H(s) = s^2 / (s^2 + s/Q + 1)
			// See "Digital Audio Signal Processing" by Udo Zölzer
			//
			a0 =  1.0 / bh;
			a1 = -2.0 / bh;
			a2 =  a0;
			b1 = (2.0 * (k2 - 1.0)) / bh;
			b2 = (1.0 - rk + k2) / bh;
			break;
		
		case Type::kBandPass:
			//
			// Bilinear transformation of H(s) = (s/Q) / (s^2 + s/Q + 1)
			// See "Digital Audio Signal Processing" by Udo Zölzer
			//
			a0 =  rk / bh;
			a1 =  0.0;
			a2 = -rk / bh;
			b1 = (2.0 * (k2 - 1.0)) / bh;
			b2 = (1.0 - rk + k2) / bh;
			break;
			
		case Type::kBandStop:
			//
			// "Digital Audio Signal Processing" by Udo Zölzer does not provide z-transform
			// coefficients for the bandstop filter, so these were derived by studying
			// http://www.earlevel.com/main/2012/11/26/biquad-c-source-code/
			//
			a0 = (1.0 + k2) / bh;
			a1 = (2.0 * (k2 - 1.0)) / bh;
			a2 =  a0;
			b1 =  a1;
			b2 = (1.0 - rk + k2) / bh;
			break;
			
		default:
			assert(nullptr == "invalid FilterType");
			return;
	}

	switch (slope) {
		case Slope::k12:
			for (int i=0; i<numSamples; i++) { double y, x = buffer[i];

				y  =      (a0 * x) + d1;
				d1 = d2 + (a1 * x) - (b1 * y);
				d2 =      (a2 * x) - (b2 * y);

				buffer[i] = (float) y;
			}
			break;

		case Slope::k24:
			for (int i=0; i<numSamples; i++) { double y, x = buffer[i];

				y  =      (a0 * x) + d1;
				d1 = d2 + (a1 * x) - (b1 * y);
				d2 =      (a2 * x) - (b2 * y);

				x = y;

				y  =      (a0 * x) + d3;
				d3 = d4 + (a1 * x) - (b1 * y);
				d4 =      (a2 * x) - (b2 * y);

				buffer[i] = (float) y;
			}
			break;

		default:
			assert(nullptr == "invalid FilterSlope");
			break;
	}
}
