/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "SoftLimiter.h"
#include <math.h>
#include <iostream>

#define AT 0.001		// attack time in seconds
#define RT 0.5			// release time in seconds
#define THRESHOLD 0.9	// THRESHOLD>0 !!

SoftLimiter::SoftLimiter(float rate)
{
	xpeak=0;
	attack=1-exp(-2.2/(AT*rate));
	release=1-exp(-2.2/(RT*rate));
	thresh=(float)log(THRESHOLD); // thresh in linear scale :)
	ch=1;
}

SoftLimiter::~SoftLimiter()
{
}

void
SoftLimiter::Process	(float *l, float *r, unsigned nframes)
{
	register double x;
	register unsigned i;
	for (i=0; i<nframes; i++)
	{
		x = fabs(*l) + fabs (*r);
		
		if (x>xpeak) xpeak=(1-release)*xpeak + attack*(x-xpeak);
		else xpeak=(1-release)*xpeak;
			
		if (xpeak>0){
//			x = 1/xpeak;
			x = log(xpeak);
			x -= thresh;
			if (x<0) x = 0;
//			x *= ((1/Ratio)-1); 
			/* 1<= Ratio < infinity = compressor
			   ratio=infinity = limiter
			   0 < ratio < 1 = expander
			   0 = gate
			*/
			else x *= -1;
			x = exp(x);
		} else x=1;
		
		*l++ *= x;
		*r++ *= x;
	}
}
