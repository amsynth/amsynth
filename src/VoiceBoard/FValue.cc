/* amSynth
 * (c) 2001,2002 Nick Dowell
 **/

#include "FValue.h"

FValue::FValue()
{
	_value = 0.0;
	_buffer = new float[BUF_SIZE];
	getvalfunc = &FValue::getIntVal;
}

FValue::~FValue()
{
  delete[]_buffer;
}

void
 FValue::setValue(float value)
{
  _value = value;
}

float FValue::getIntVal()
{
  return _value;
}

float FValue::getValue()
{
  return (this->*getvalfunc) ();
}

void FValue::setParameter(Parameter & param)
{
#ifdef _DEBUG
  cout << "<FValue> using Parameter: '" << param.
	getName() << "'" << endl;
#endif
  _param = &param;
  getvalfunc = &FValue::getParamVal;
}

float FValue::getParamVal()
{
  return _param->getControlValue();
}

float *FValue::getFData()
{
	float tmp = getValue();
	register int i;
	for (i = 0; i < BUF_SIZE; i++) 
		_buffer[i] = tmp;
	return _buffer;
}
