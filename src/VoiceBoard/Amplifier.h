/* amSynth
 * (c) 2001,2002 Nick Dowell
 **/

#ifndef _AMPLIFIER_H
#define _AMPLIFIER_H

#include "Synth--.h"

/**
 * @class Amplifier
 * A simple (Audio) signal level amplifier
 * for general purpose use, use the Multiplier class in preference - it can 
 * handle more than 2 inputs..
 * The Amplifier class, however, is an NFSource, and is tailored for amplifying
 * an audio stream. As such it may have optimisations which exploit its intended
 * use..
 **/
class Amplifier : public NFSource, public NFInput {

public:
	Amplifier(float *buf);
	virtual ~Amplifier();
	/**
	 * @param source the NFSource to use as the (audio) input
	 */
	void setInput(NFSource & source);
	/**
	 * @param source the NFSource which will control the volume. Do not set
	 * this as the audio stream, as optimisations may lead to strange artifacts.
	 */
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
