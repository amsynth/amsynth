/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#ifndef _LOWPASSFILTER_H
#define _LOWPASSFILTER_H

/**
 * A 24 dB/ocatave resonant low-pass filter.
 **/
class LowPassFilter
{
public:
	LowPassFilter();

	void	SetSampleRate	(int rateIn) { rate = (float) rateIn; nyquist = rate/(float)2; }
	
	/**
	 * Reset the filter - clear anything in the delay units of the filter.
	 */
	void reset();

	void	ProcessSamples	(float*, int, float cutoff, float res);
private:
	float rate;
	float nyquist;
	double d1, d2, d3, d4;
};

#endif
