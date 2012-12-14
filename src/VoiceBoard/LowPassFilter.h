/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#ifndef _LOWPASSFILTER_H
#define _LOWPASSFILTER_H

class SynthFilter
{
public:

	enum FilterType {
		FilterTypeLowPass,
		FilterTypeHighPass,
		FilterTypeBandPass,
		FilterTypeCount
	};

	enum FilterSlope {
		FilterSlope12,
		FilterSlope24,
	};

	SynthFilter();

	void SetSampleRate(int rateIn) { rate = (float)rateIn; nyquist = rate / 2.0f; }

	void reset();

	void ProcessSamples(float *, int, float cutoff, float res, FilterType type, FilterSlope slope);

private:

	float rate;
	float nyquist;
	double d1, d2, d3, d4;
};

#endif
