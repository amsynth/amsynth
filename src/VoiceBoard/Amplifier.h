/* amSynth
 * (c) 2001,2002 Nick Dowell
 **/

#ifndef _AMPLIFIER_H
#define _AMPLIFIER_H

#include "Synth--.h"

/**
 * @class Amplifier
 * use the Multiplier class in preference - it can handle more than 2 inputs..
 **/
class Amplifier : public NFSource, public NFInput {

public:
	Amplifier();
	virtual ~Amplifier();
	void setInput(NFSource & source);
	void setCInput(NFSource & source);
	inline float *getNFData();
private:
	NFSource * source1;
	NFSource * source2;
	float *buffer;
	float *buffer1;
	float *buffer2;
};

#endif				// _AMPLIFIER_H
