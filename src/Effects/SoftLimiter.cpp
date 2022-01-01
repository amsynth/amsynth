/*
 *  SoftLimiter.cpp
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

#include "SoftLimiter.h"
#include <math.h>
#include <iostream>

#define AT 0.0001		// attack time in seconds
#define RT 0.5			// release time in seconds
#define THRESHOLD 0.9	// THRESHOLD>0 !!

void
SoftLimiter::SetSampleRate	(int rate)
{
	xpeak=0;
	attack=1-exp(-2.2/(AT*(float)rate));
	release=1-exp(-2.2/(RT*(float)rate));
	thresh=(float)log(THRESHOLD); // thresh in linear scale :)
}

void
SoftLimiter::Process	(float *l, float *r, unsigned nframes, int stride)
{
	double x;
	unsigned i;
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
		
		// apply gain
		*l *= x;
		*r *= x;
		
		// Increment sample pointers, allowing for interleave (if any)
		l += stride;
		r += stride;
	}
}
