/* amSynth
 * (c) 2002 Nick Dowell
 **/

#include "FilterControlSignal.h"

const int j = (int)(BUF_SIZE/2);

void
FilterControlSignal::setLFO( FSource & source )
{
	lfo = &source;
}

void	
FilterControlSignal::setEnvelope( NFSource & source )
{
	env_source = &source;
}

void
FilterControlSignal::setKeyPitch( FSource & source )
{
	pitch_source = &source;
}

void
FilterControlSignal::setVelocity( const float val )
{
	vel = val;
}

void
FilterControlSignal::setModAmount( Parameter & param )
{
	mod_param = &param;
	param.addUpdateListener( *this );
}

void
FilterControlSignal::setEnvAmount( Parameter & param )
{
	env_param = &param;
	param.addUpdateListener( *this );
}

void
FilterControlSignal::setCutoffControl( Parameter & param )
{
	cutoff_param = &param;
	param.addUpdateListener( *this );
}

void
FilterControlSignal::update()
{
	env_amount = env_param->getControlValue();
	mod_amount = (mod_param->getControlValue()+1.0)/2.0;
	cutoff = cutoff_param->getControlValue();
}

inline void
FilterControlSignal::process()
{
	env_buf = env_source->getNFData();
	lfo_buf = lfo->getFData();
	pitch_buf = pitch_source->getFData();
	
	/* we can cheat - the filter (currently) only uses the cutoff value in
	 * one element of the buffer (j)
	 */
	env_buf[j] = pitch_buf[j] * env_buf[j] * env_amount 
			+ ( pitch_buf[j] * vel * cutoff ) * 
			( (lfo_buf[j]*0.5 + 0.5) * mod_amount + 1-mod_amount );
}