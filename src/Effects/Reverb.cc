/* amSynth
 * (c) 2001-2004 Nick Dowell
 */

#include "Reverb.h"

Reverb::Reverb()
{
//	model.mute();
}

void
Reverb::Process	(float *in, float *l, float *r, unsigned nframes)
{
	model.processreplace (in, in, l, r, nframes, 1);
}
