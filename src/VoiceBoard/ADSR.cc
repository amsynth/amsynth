/**
 * @file   ADSR.cc
 * @brief  Implementation of an ADSR contour generator
 *
 * @author Nick Dowell
 * @date   2001-11-25
*/
#include "ADSR.h"

ADSR::ADSR(int rate, float *buf)
{
    this->rate = rate;
    state = off;
    c_val = 0;

    buffer = buf;
}

void
ADSR::triggerOn()
{
	float a_time=(1.0/((float)a_delta*rate));
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

void
ADSR::reset()
{
	state = off;
	c_val = 0;
}

void 
ADSR::SetAttack		(float val)
{
	if (val == 0.0)	a_delta = 1;
	else		a_delta = 1 / (val * (float) rate);
}

void 
ADSR::SetDecay		(float val)
{
	d_frames = val * rate;
	if (val == 0)	d_delta = 1;
	else		d_delta = 1 / (val * (float) rate);
}

void 
ADSR::SetSustain	(float val)
{
	s_val = val;
}

void 
ADSR::SetRelease	(float val)
{
	r_time = val;
	if (r_time == 0.0) r_time = 0.001;
}

int 
ADSR::getState()
{
	return (state == off) ? 0 : 1;
}

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
				inc=(1.0-c_val)/(float)nFrames;
				state = decay;
				m_decay_frames=d_frames;
			}
			break;
		case decay:
			inc=(s_val-1.0)/(float)d_frames; m_decay_frames-=nFrames;
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
