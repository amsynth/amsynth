/* amSynth
 * (c) 2001 Nick Dowell
 **/

#include "NFValue.h"

NFValue::NFValue()
{
    _value = 0.0;
    _buffer = new float[BUF_SIZE];
    getvalfunc = &NFValue::getIntVal;
}

NFValue::~NFValue()
{
    delete[]_buffer;
}

void
NFValue::setValue(float value)
{
    _value = value;
}

float 
NFValue::getIntVal()
{
    return _value;
}

float 
NFValue::getValue()
{
    return (this->*getvalfunc) ();
}

void 
NFValue::setParameter(Parameter & param)
{
#ifdef _DEBUG
    cout << "<NFValue> using Parameter: '" << param.
	getName() << "'" << endl;
#endif
    _param = &param;
    getvalfunc = &NFValue::getParamVal;
}

float 
NFValue::getParamVal()
{
	return _param->getControlValue();
}

float *
NFValue::getNFData()
{
	register float tmp = getValue();
	register int i;
	for (i = 0; i < BUF_SIZE; i++) _buffer[i] = tmp;
	return _buffer;
}
