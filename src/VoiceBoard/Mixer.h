/* amSynth
 * (c) 2001 Nick Dowell
 **/

#ifndef _MIXER_H
#define _MIXER_H

#include "Synth--.h"
#include "../Parameter.h"
#include "../UpdateListener.h"
#define MAX_INPUTS 128

/**
 * @class Mixer
 * @brief A mixer object. takes two input NFSources --(+)--> one NFSource output
 * it allows you to control the relative strenghts of the two signals in the output.
 */

class Mixer : public NFSource, public UpdateListener {
public:
	Mixer();
	virtual ~Mixer();
	void setInput1(FSource & source);
	void setInput2(FSource & source);
	/**
	 * for the control input NFSource, -1 indicates 100% on input1, 0% input2.
	 * 1 indicates 0% input1, 100% input2. 0 = 50% each
	 */
	void setControl(NFSource & source);
	void setMode( Parameter & param );
	void update();
	inline float * getNFData();

private:
	float *buffer;
	float *inBuffer1, *inBuffer2, *controlBuffer;
	int mix_mode;
	FSource *input1, *input2;
	NFSource *control;
	Parameter *mode_param;
};

#endif
