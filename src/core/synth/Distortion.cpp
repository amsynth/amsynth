/*
 *  Distortion.cpp
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

void 
Distortion::SetCrunch	(float value)
{
	crunch=1-value;
}

void
Distortion::Process	(float *buffer, unsigned nframes)
{
	float x, s;
	for (unsigned i=0; i<nframes; i++)
	{
		x = buffer[i];
		if(x<0) s=-1; else s=1;
		x*=s;
		float c = crunch.tick();
		x = pow (x, c < 0.01f ? 0.01f : c);
		buffer[i] = x*s;
	}
}
