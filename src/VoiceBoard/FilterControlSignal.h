/* amSynth
 * (c) 2002 Nick Dowell
 **/

#ifndef _FILTER_CONTROL_SIGNAL_H
#define _FILTER_CONTROL_SIGNAL_H

#include "Synth--.h"
#include "../UpdateListener.h"
#include "../Parameter.h"

class FilterControlSignal : public FSource, public UpdateListener {
public:
	void		setLFO( FSource & source );
	void		setEnvelope( NFSource & source );
	void		setKeyPitch( FSource & source );
	void		setVelocity( const float val );
	void		setModAmount( Parameter & param );
	void		setEnvAmount( Parameter & param );
	void		setCutoffControl( Parameter & param );
	
	void		update();
	inline void	process();
	inline float*	getFData(){ return env_buf; };
private:
	NFSource	*env_source;
	FSource		*pitch_source, *lfo;
	Parameter	*mod_param, *env_param, *cutoff_param;
	float		*env_buf, *pitch_buf, *lfo_buf;
	float		mod_amount, env_amount, cutoff, vel;
};

#endif
