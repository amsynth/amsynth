/**
 * @file   ADSR.cc
 * @brief  Implementation of an ADSR contour generator
 *
 * @author Nick Dowell
 * @date   2001-11-25
*/
#include <stdio.h>

#include "ADSR.h"

#define ADSR_OFF 0
#define ADSR_A   1
#define ADSR_D   2
#define ADSR_S   3
#define ADSR_R   4

ADSR::ADSR(int rate, float *buf)
{
    this->rate = rate;
    state = 0;
    c_val = 0;

    buffer = buf;
}

void
ADSR::triggerOn()
{
	float a_time=(1.0/((float)a_delta*rate));
	m_attack_frames=a_time*rate;
    state = ADSR_A;
}

void 
ADSR::triggerOff()
{
	m_release_frames=r_time*rate;
	r_delta = -c_val/(float)m_release_frames;
    state = ADSR_R;
}

void
ADSR::reset()
{
	state = ADSR_OFF;
	c_val = 0;
}

void 
ADSR::setAttack( Parameter & param )
{
    attackParam = &param;
	param.addUpdateListener( *this );
	update();
}

void 
ADSR::setDecay( Parameter & param )
{
	decayParam = &param;
	param.addUpdateListener( *this );
	update();
}

void 
ADSR::setSustain( Parameter & param )
{
    sustainParam = &param;
	param.addUpdateListener( *this );
	update();
}

void 
ADSR::setRelease( Parameter & param )
{
    releaseParam = &param;
	param.addUpdateListener( *this );
	update();
}

void
ADSR::update()
{
	if(attackParam){
		if(attackParam->getControlValue()==0)
			a_delta = 1;
		else
			a_delta = 1/( attackParam->getControlValue() * rate );
	}
	if(decayParam)
	{
		d_frames=decayParam->getControlValue()*rate;
		if(decayParam->getControlValue()==0)
			d_delta = 1;
		else
			d_delta = 1/( decayParam->getControlValue() * rate );
	}
	if(sustainParam)
		s_val = sustainParam->getControlValue();
	if(releaseParam)
		r_time = releaseParam->getControlValue();
	if(r_time==0)
		r_time = 0.001;
}

int 
ADSR::getState()
{
    if (state == ADSR_OFF)
		return 0;
    else
		return 1;
}

float *
ADSR::getNFData()
{
	register int i;
	register float inc;
	
	switch(state)
	{
		case ADSR_A:
			inc=a_delta; m_attack_frames-=BUF_SIZE;
			if (m_attack_frames<=0)
			{
				inc=(1.0-c_val)/(float)BUF_SIZE;
				state=ADSR_D;
				m_decay_frames=d_frames;
			}
			break;
		case ADSR_D:
			inc=(s_val-1.0)/(float)d_frames; m_decay_frames-=BUF_SIZE;
			if (m_decay_frames<=0)
			{
				inc=-(c_val-s_val)/(float)BUF_SIZE;
				state=ADSR_S;
			}
			break;
		case ADSR_S:
			c_val=s_val; inc=0.0; break;
		case ADSR_R:
			inc=r_delta; m_release_frames-=BUF_SIZE;
			if (m_release_frames<=0)
			{
				inc=c_val/(float)BUF_SIZE;
				state=ADSR_OFF;
			}
			break;
		default:
			inc=0.0; c_val=0.0; break;
	}
	i=0; while (i<BUF_SIZE) { buffer[i++] = c_val; c_val+=inc; }
	return buffer;
}
