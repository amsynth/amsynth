/* amSynth
 * (c) 2002 Nick Dowell
 */

#ifndef _AMP_SECTION_H
#define _AMP_SECTION_H

#include "Synth--.h"
#include "../UpdateListener.h"
#include "../Parameter.h"

class AmpSection : public NFSource, public UpdateListener {
public:
	// input:
	void		setInput( NFSource & source );

	// control signals:
	void		setLFO( FSource & source );
	void		setEnvelope( NFSource & source );

	// control values:
	void		setVelocity( float val ){ vel=val; };
	void		setModAmount( Parameter & param );

	void		update();
	inline float*	getNFData();
private:
	NFSource	*input, *env;
	FSource		*lfo;
	float		*lfo_buf, *env_buf, *buffer;
	float		vel, mod_amount;
	Parameter	*mod_amount_param;
};

#endif
