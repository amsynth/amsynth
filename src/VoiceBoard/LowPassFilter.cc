/* amSynth
 * (c) 2001-2004 Nick Dowell
 */

#include "LowPassFilter.h"
#include <math.h>

LowPassFilter::LowPassFilter(int rate)
{
	f = k = p = r = d1 = d2 = d3 = d4 = 0.0;
	a0 = a1 = a2 = b1 = b2 = 0.0;
	
	this->rate = rate;
	nyquist = rate/(float)2;
	max = 1;
}

void
LowPassFilter::reset()
{
	d1 = d2 = d3 = d4 = 0;
}

void
LowPassFilter::Process64Samples	(float *buffer, float fc, float res)
{
	// constrain cutoff
#define SAFE 0.99 // filter is unstable _AT_ PI
	if (fc>(nyquist*SAFE))
		fc=nyquist*SAFE;
	if (fc<10)
		{fc = 10;/*d1=d2=d3=d4=0;*/}
	float w = (fc/(float)rate); // cutoff freq [ 0 <= w <= 0.5 ]
	
	// find final coeff values for end of this buffer
	register double k, k2, bh, a0, a1, a2, b1, b2;
	double r = 2*(1-res);
	if(r==0.0) r = 0.001;
	k=tan(w*PI);
	k2 = k*k;
	bh = 1 + (r*k) + k2;
	a0 = a2 = double(k2/bh);
	a1 = a0 * 2;
	b1 = double(2*(k2-1)/-bh);
	b2 = double((1-(r*k)+k2)/-bh);
	
	double x,y;
	for (int i=0; i<64; i++) {
		x = buffer[i];
		
		// first 2nd-order unit
		y = ( a0*x ) + d1;
		d1 = d2 + ( (a1)*x ) + ( (b1)*y );
		d2 = ( (a2)*x ) + ( (b2)*y );
		x=y;
		// and the second
		
		y = ( a0*x ) + d3;
		d3 = d4 + ( a1*x ) + ( b1*y );
		d4 = ( a2*x ) + ( b2*y );
		
		buffer[i] = y;
	}
}
