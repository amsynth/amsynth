/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#ifndef _DISTORTION_H
#define _DISTORTION_H

#include "Synth--.h"
#include "../UpdateListener.h"
#include "../Parameter.h"

/**
 * @brief A distortion (waveshaping) effect unit
 */
class Distortion: public NFSource, public FInput, public UpdateListener
{
public:
	Distortion();
	void setInput( FSource & input );
	inline float * getNFData();
	/**
	 * @param parameter The Parameter to control the 'drive' of the effect - 
	 * i.e. the input gain.
	 */
	void setDrive( Parameter & parameter );
	/**
	 * @param parameter The Parameter to control the 'crunch' of the effect - 
	 * the amount of waveshape distortion.
	 */
	void setCrunch( Parameter & parameter );
	void update();
private:
	FSource *input;
	Parameter *driveParam;
	Parameter *crunchParam;
	float *buffer;
	float drive, crunch;
	int done;
};

#endif
