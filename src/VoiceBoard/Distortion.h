/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#ifndef _DISTORTION_H
#define _DISTORTION_H

#include "Synth--.h"
#include "../UpdateListener.h"
#include "../Parameter.h"

class Distortion: public NFSource, public FInput, public UpdateListener
{
public:
	Distortion();
	void setInput( FSource & input );
	inline float * getNFData();
	void setDrive( Parameter & parameter );
	void setCrunch( Parameter & parameter );
	void update();
private:
	FSource *input;
	Parameter *driveParam;
	Parameter *crunchParam;
	float *buffer;
	float drive, crunch;
};

#endif
