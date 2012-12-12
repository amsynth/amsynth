/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#include "LowPassFilter.h"
#include "Synth--.h"		// for PI and TWO_PI

#include <algorithm>
#include <cassert>
#include <math.h>

SynthFilter::SynthFilter() :
	rate (4100.0)
,	nyquist (22050.0)
{
}

void
SynthFilter::reset()
{
	d1 = d2 = d3 = d4 = 0;
}

void
SynthFilter::ProcessSamples(float *buffer, int numSamples, float cutoff, float res, FilterType filterType)
{
#define SAFE 0.99f // filter is unstable _AT_ PI
	cutoff = std::min(cutoff, nyquist * SAFE);
	cutoff = std::max(cutoff, 10.0f);

	float w = (cutoff / rate); // cutoff freq [ 0 <= w <= 0.5 ]
	
	const double k = tan(w * PI);
	const double k2 = k * k;
	const double r = std::max(0.001, 2.0 * (1.0 - res)); // r is 1/Q (sqrt(2) for a butterworth response)

	const double bh = 1.0 + r * k + k2;

	double a0, a1, a2, b1, b2;

	switch (filterType) {
		case FilterTypeLowPass:
			//
			// Bilinear transformation of H(s) = 1 / (s^2 + s/Q + 1)
			// See "Digital Audio Signal Processing" by Udo Zölzer
			//
			a0 = k2 / bh;
			a1 = a0 * 2.0;
			a2 = a0;
			b1 = (2.0 * (k2 - 1.0))   / bh;
			b2 = (1.0 - (r * k) + k2) / bh;
			break;
		case FilterTypeHighPass:
			//
			// Bilinear transformation of H(s) = s^2 / (s^2 + s/Q + 1)
			// See "Digital Audio Signal Processing" by Udo Zölzer
			//
			a0 =  1.0 / bh;
			a1 = -2.0 / bh;
			a2 =  a0;
			b1 = (2.0 * (k2 - 1.0))   / bh;
			b2 = (1.0 - (r * k) + k2) / bh;
			break;
		case FilterTypeBandPass:
			//
			// Bilinear transformation of H(s) = (s/Q) / (s^2 + s/Q + 1)
			// See "Digital Audio Signal Processing" by Udo Zölzer
			//
			a0 =  r * k / bh;
			a1 =  0.0;
			a2 = -r * k / bh;
			b1 = (2.0 * (k2 - 1.0))   / bh;
			b2 = (1.0 - (r * k) + k2) / bh;
			break;
		default:
			assert(!"invalid FilterType");
			return;
	}

	double x,y;
	for (int i=0; i<numSamples; i++) {
		x = buffer[i];

		// Two direct form 2 biquads

		y  =      (a0 * x) + d1;
		d1 = d2 + (a1 * x) - (b1 * y);
		d2 =      (a2 * x) - (b2 * y);

		x = y;

		y  =      (a0 * x) + d3;
		d3 = d4 + (a1 * x) - (b1 * y);
		d4 =      (a2 * x) - (b2 * y);
		
		buffer[i] = (float) y;
	}
}
