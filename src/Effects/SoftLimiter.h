/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#ifndef _SOFTLIMITER_H
#define _SOFTLIMITER_H

class SoftLimiter
{
public:
	void	SetSampleRate	(int rate);
	void	isStereo(){ch=2;};
	void	Process	(float *l, float *r, unsigned);
  private:
    float *buffer;
	double xpeak, attack, release, thresh;
	int ch;
};

#endif
