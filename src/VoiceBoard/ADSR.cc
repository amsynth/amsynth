/*
 *  ADSR.cc
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

#include "ADSR.h"

ADSR::ADSR(float * const buf):
	buffer (buf)
,	state (off)
,	c_val (0.0)
{}

void
ADSR::SetSampleRate	(int rateIn)
{
	rate = rateIn;
	SetAttack (a_time);
	SetDecay (d_time);
}

void
ADSR::triggerOn()
{
	if (a_time == 0.0)	a_delta = 1;
	else				a_delta = 1 / (a_time * (float) rate);

	m_attack_frames=a_time*rate;
	state = attack;
}

void 
ADSR::triggerOff()
{
	m_release_frames=r_time*rate;
	r_delta = -c_val/(float)m_release_frames;
	state = release;
}

void ADSR::reset		()			{ state = off; c_val = 0; }
void ADSR::SetAttack	(float val)	{ a_time = val; }
void ADSR::SetDecay		(float val)	{ d_time = val; }
void ADSR::SetSustain	(float val)	{ s_val = val; }
void ADSR::SetRelease	(float val) { r_time = val;	if (r_time == 0.0f) r_time = 0.001f; }
int  ADSR::getState		()			{ return (state == off) ? 0 : 1; }

float * 
ADSR::getNFData(int nFrames)
{
	register int i;
	register float inc;
	
	switch(state)
	{
		case attack:
			inc=a_delta; m_attack_frames-=nFrames;
			if (m_attack_frames<=0)
			{
				inc=(1.0f-c_val)/(float)nFrames;

				state = decay;
				d_frames = d_time * rate;
				if (d_time == 0)	d_delta = 1;
				else				d_delta = 1 / (d_time * (float) rate);
				m_decay_frames=d_frames;
			}
			break;
		case decay:
			inc=(s_val-1.0f)/(float)d_frames; m_decay_frames-=nFrames;
			if (m_decay_frames<=0)
			{
				inc=-(c_val-s_val)/(float)nFrames;
				state = sustain;
			}
			break;
		case sustain:
			c_val=s_val; inc=0.0; break;
		case release:
			inc=r_delta; m_release_frames-=nFrames;
			if (m_release_frames<=0)
			{
				inc=c_val/(float)nFrames;
				state = off;
			}
			break;
		default:
			inc=0.0; c_val=0.0; break;
	}
	i=0; while (i<nFrames) { buffer[i++] = c_val; c_val+=inc; }
	return buffer;
}
