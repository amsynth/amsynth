/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#ifndef _LIMITER_H
#define _LIMITER_H

#include "Synth--.h"

/**
 * @class Limiter
 *
 * takes an input stream, which can be NF or F, and outputs an NF stream,
 * clipping the level so it stays in range. This is the simplest way to get
 * from an F stream back to NF..
 **/
class Limiter : public NFSource, public FInput
{
public:
  Limiter();
  virtual ~Limiter();
  void setInput( FSource & source );
  inline float * getNFData();

public:
  FSource * source;
  float * buffer;
};

#endif
