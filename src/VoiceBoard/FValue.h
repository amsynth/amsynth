/* Synth--
 * (c) 2001 Nick Dowell
 **/

#ifndef _FVALUE_H
#define _FVALUE_H

#include "Synth--.h"
#include "../Parameter.h"
#ifdef _DEBUG
#include <iostream>
#endif

/** @class FValue
 *
 * an object which provides a stream of Float values..
 **/
class FValue:public FSource {
  public:
    FValue(float *buf);
    virtual ~FValue();
    inline float *getFData();
    void setValue(float value);
    inline float getValue();
    void setParameter(Parameter & param);
  private:
    float _value, *_buffer;
    float (FValue::*getvalfunc) ();
    float getIntVal();
    float getParamVal();
    Parameter *_param;
};
#endif
