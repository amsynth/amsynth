/*
 *  Distortion.cc
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
	if (crunch == 0) crunch = 0.01f;
	
	for (unsigned i=0; i<nframes; i++)
	{
		x = buffer[i]*drive;
		if(x<0) s=-1; else s=1;
		x*=s;
		x = pow (x, crunch);
		buffer[i] = x*s;
	}
}
