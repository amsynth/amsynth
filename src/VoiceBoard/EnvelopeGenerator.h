/* Synth--
 * (c) 2001 Nick Dowell
 **/
#ifndef _ENVELOPEGENERATOR_H
#define _ENVELOPEGENERATOR_H

#include "Synth--.h"


class EnvelopeGenerator : public NFSource {

  public:
    EnvelopeGenerator();
    virtual ~ EnvelopeGenerator();
    inline virtual float *getNFData();
    virtual void triggerOn();
    virtual void triggerOff();
  private:
    float value;
    float *buffer;
};


#endif				//ENVELOPEGENERATOR_H
