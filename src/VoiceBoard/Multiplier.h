/* amSynth
 * (c) 2001,2002 Nick Dowell
 **/

#ifndef _MULTIPLIER_H
#define _MULTIPLIER_H

#include "Synth--.h"
#define MAX_INPUTS 128

/**
 * @class Multiplier
 * @brief Multiplies two input streams (which can be NFSources or FSources).
 * If NFSource output is required, use the Limiter to convert the output.
 */

class Multiplier : public FSource {
public:
	Multiplier();
	virtual ~Multiplier();
	void addInput(FSource & source);
	void removeInput(FSource & source);
	inline float *getFData();
private:
	float *_buffer;
	float *inBuffer;
	FSource *inputs[MAX_INPUTS];
	int inputExists[MAX_INPUTS], no_of_inputs;
};

#endif

