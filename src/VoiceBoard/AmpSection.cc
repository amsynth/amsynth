/* amSynth
 * (c) 2002 Nick Dowell
 */

#include "AmpSection.h"

void
AmpSection::setLFO( FSource & source )
{
	lfo = &source;
}

void
AmpSection::setInput( NFSource & source )
{
	input = &source;
}

void
AmpSection::setEnvelope( NFSource & source )
{
	env = &source;}

void
AmpSection::setModAmount( Parameter & param )
{
	mod_amount_param = &param;
	param.addUpdateListener( *this );
}

void
AmpSection::update()
{
	mod_amount = (mod_amount_param->getControlValue()+1.0)/2.0;
}

inline float*
AmpSection::getNFData()
{
	buffer = input->getNFData();
	env_buf = env->getNFData();
	lfo_buf = lfo->getFData();
	
	register int i;
	for( i=0; i<BUF_SIZE; i++ ) 
		buffer[i] = buffer[i]*env_buf[i]*vel *
			( ((lfo_buf[i]*0.5)+0.5)*mod_amount + 1-mod_amount);
	
	return buffer;
}

