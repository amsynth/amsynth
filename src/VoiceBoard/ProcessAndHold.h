/* amSynth
 * (c)2001,2002 Nick Dowell
 */

#ifndef _PROCESSANDHOLD_H
#define _PROCSESANDHOLD_H

#include "Synth--.h"

class ProcessAndHold : public FSource, public FInput
{
public:
	ProcessAndHold();
	virtual ~ProcessAndHold();
	inline float * getFData();
	void setInput( FSource & source );
	inline void process();
private:
	FSource * input;
	float * buffer;
};

#endif