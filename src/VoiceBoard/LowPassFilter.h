/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#ifndef _LOWPASSFILTER_H
#define _LOWPASSFILTER_H

#include "Synth--.h"
#include "../UpdateListener.h"
#include "../Parameter.h"

/**
 * @class LowPassFilter
 **/
class LowPassFilter : public NFSource, public NFInput, public UpdateListener
{
public:
	LowPassFilter(int rate);
	virtual ~LowPassFilter(){};
	inline float * getNFData();
	void setInput( NFSource & source );
	void setCFreq( FSource & fval );
	void setCutoff( Parameter & param );
	void setResonance( Parameter & param );
	void reset();
	void update();
private:
	Parameter * cutoff_param;
	Parameter * res_param;
	NFSource * source;
	FSource * cutoff;
	int rate;
	float nyquist;
	double a0, a1, a2, b1, b2, res;
	float f, k, p, r, max;
	double d1, d2, d3, d4, x, y;
};

#endif