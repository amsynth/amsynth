/* amSynth
 * (c) 2001-2004 Nick Dowell
 */

#ifndef _UPDATELISTENER_H
#define _UPDATELISTENER_H

#include "controls.h"

/**
 * an interface for classes which can be update() ed, eg the GUI objects.
 **/
class UpdateListener {
  public:
    virtual void update			()		{;}
    virtual void UpdateParameter	(Param, float)	{ update ();}
};

#endif
