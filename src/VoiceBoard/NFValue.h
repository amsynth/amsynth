/* amSynth
 * (c) 2001 Nick Dowell
 **/

#ifndef _NFVALUE_H
#define _NFVALUE_H

#include "Synth--.h"
#include "../Parameter.h"
#ifdef _DEBUG
#include <iostream>
#endif

/** @class NFValue
 *
 * an object which provides a stream of Normalised Float values..
 **/
class NFValue:public NFSource {
  public:
    NFValue();
    virtual ~NFValue();
    inline float *getNFData();
    void setValue(float value);
    inline float getValue();
    void setParameter(Parameter & param);
  private:
    float _value;
    float *_buffer;
    float (NFValue::*getvalfunc) ();
    float getIntVal();
    float getParamVal();
    Parameter *_param;
};
#endif
