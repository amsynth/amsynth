/* amSynth
 * (c) 2001-2004 Nick Dowell
 */

#ifndef _LOWPASSFILTER_H
#define _LOWPASSFILTER_H

#include "Synth--.h"

/**
 * A 24 dB/ocatave resonant low-pass filter.
 **/
class LowPassFilter
{
public:
	LowPassFilter(int rate);
	virtual ~LowPassFilter(){};
	
	/**
	 * Reset the filter - clear anything in the delay units of the filter.
	 */
	void reset();

	void	ProcessSamples	(float*, int, float cutoff, float res);
private:
	int rate;
	float nyquist;
	double a0, a1, a2, b1, b2, res;
	float f, k, p, r, max;
	double d1, d2, d3, d4, x, y;
};

#endif
