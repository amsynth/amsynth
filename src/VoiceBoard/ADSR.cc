/**
 * @file   ADSR.cc
 * @brief  Implementation of an ADSR contour generator
 *
 * @author Nick Dowell
 * @date   2001-11-25
*/
#include <iostream>

#include "ADSR.h"

#define ADSR_OFF 0
#define ADSR_A   1
#define ADSR_D   2
#define ADSR_S   3
#define ADSR_R   4

ADSR::ADSR(int rate)
{
    this->rate = rate;
    state = 0;
    c_val = 0;

    buffer = new float[BUF_SIZE];
}

ADSR::~ADSR()
{
    delete[] buffer;
}

void
ADSR::triggerOn()
{
    state = ADSR_A;
}

void 
ADSR::triggerOff()
{
	r_delta = c_val / (r_time*rate);
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
		if(decayParam->getControlValue()==0)
			d_delta = 1;
		else
			d_delta = 1/( decayParam->getControlValue() * rate );
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
    // this could really do with cleaning up / optimisation..
	register int i;
    for (i = 0; i < BUF_SIZE; i++) {
		switch (state) {
	
		case ADSR_OFF:
//			c_val = 0;
			buffer[i] = 0.0;
			break;
	
		case ADSR_A:
			buffer[i] = (c_val += a_delta);
			if (c_val >= 1.0) {
				state = ADSR_D;
				c_val = 1.0;
			}
			break;
	
		case ADSR_D:
			if ( c_val <= s_val ) {
				buffer[i] = (c_val = s_val);
				if( s_val > 0.001 ) state = ADSR_S;
				else state = ADSR_OFF;
			} else {
				buffer[i] = (c_val -= d_delta);
			}
			break;
	
		case ADSR_S:
			buffer[i] = s_val;
			break;
	
		case ADSR_R:
			c_val -= r_delta;
			if (c_val < r_delta) {
				c_val = 0;
				state = ADSR_OFF;
			}
			buffer[i] = c_val;
			break;
	
		default:
#ifdef _DEBUG
			cout << "<ADSR> state error" << endl;
#endif
			break;
		}
    }
    return buffer;
}
