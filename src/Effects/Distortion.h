/* amSynth
 * (c) 2001-2004 Nick Dowell
 */

#ifndef _DISTORTION_H
#define _DISTORTION_H

/**
 * @brief A distortion (waveshaping) effect unit
 */
class Distortion
{
public:
	Distortion();

	void	SetCrunch		(float);
	void	Process			(float *buffer, unsigned);
private:
	float drive, crunch;
	int done;
};

#endif
