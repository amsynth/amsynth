/* amSynth
 * (c) 2001 Nick Dowell
 */

#ifndef _UPDATELISTENER_H
#define _UPDATELISTENER_H

/**
 * an interface for classes which can be update() ed, eg the GUI objects.
 **/
class UpdateListener {
  public:
    virtual void update()
    = 0;
};

#endif
