/* amSynth
 * (c) 2001-2004 Nick Dowell
 */

#ifndef _REVERB_H
#define _REVERB_H

#include "revmodel.hpp"

class Reverb
{
  public:
    Reverb();

	void	Alloc	(int nFrames);
    
	void	SetRoomSize	(float val)	{ model.setroomsize (val); }
	void 	SetDamp		(float val)	{ model.setdamp (val); }
	void 	SetWet		(float val)	{ model.setwet (val); model.setdry (1.0-val); }
	void 	SetWidth	(float val)	{ model.setwidth (val); }
	void 	SetMode		(float val)	{ model.setmode (val); }
	void mute()
	{ model.mute(); };

	void	Process		(float *in, float *l, float *r, unsigned);

  private:
	revmodel model;
};

#endif
