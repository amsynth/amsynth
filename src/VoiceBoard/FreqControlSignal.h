/* amSynth
 * (c) 2002 Nick Dowell
 **/

#ifndef _FREQ_CONTROL_SIGNAL
#define _FREQ_CONTROL_SIGNAL

#include "Synth--.h"
#include "../UpdateListener.h"
#include "../Parameter.h"

class FreqControlSignal : public FSource, public UpdateListener {
public:
					FreqControlSignal();
	void			setLFO( FSource &source );
	void			setPitchBend( FSource & source );
	void			setKeyPitch( FSource & source );
	void			setModAmount( Parameter & param );
	void			update();
	inline void		process();
	inline float*		getFData(){ return buffer; };
private:
	FSource*	lfo;
	FSource*	pitch_bend_source;
	FSource*	key_pitch_source;
	Parameter*	mod_amount_param;
	float		mod_amount;
	float*		buffer;
	float*		lfo_buf;
};

#endif
