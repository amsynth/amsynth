/* amSynth
 * (c) 2001-2004 Nick Dowell
 */

#include "Distortion.h"
#include <math.h>

Distortion::Distortion()
{
    drive = 1;
    crunch = 1 / 4;
	done = 0;
}

void 
Distortion::SetCrunch	(float value)
{
	crunch=1-value;
}

void
Distortion::Process	(float *buffer, unsigned nframes)
{
	register float x, s;
	if (crunch == 0) crunch = 0.01;
	
	for (unsigned i=0; i<nframes; i++)
	{
		x = buffer[i]*drive;
		if(x<0) s=-1; else s=1;
		x*=s;
		x = pow (x, crunch);
		buffer[i] = x*s;
	}
}
