/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#include "Parameter.h"

#include <cstdlib>

#ifdef _DEBUG
#include <iostream>
using namespace std;
#endif

Parameter::Parameter	(string name, Param id, float value, float min, float max, float inc, ControlType type, float base, float offset, string label)
:	mParamId	(id)
,	_name		(name)
,	_label		(label)
,	_controlMode	(type)
,	_min		(min)
,	_max		(max)
,	_step		(inc)
,	_controlValue(0)
,	_base		(base)
,	_offset		(offset)
{
	setValue (value);
}

void
Parameter::addUpdateListener	(UpdateListener& ul)
{
	for (unsigned i=0; i<_updateListeners.size(); i++) if (_updateListeners[i] == &ul) return;
	_updateListeners.push_back (&ul);
	_updateListeners.back()->UpdateParameter (mParamId, _controlValue);
}

void
Parameter::removeUpdateListener( UpdateListener & ul )
{
	for (unsigned i=0; i<_updateListeners.size(); i++)
		if (_updateListeners[i] == &ul) _updateListeners.erase(_updateListeners.begin()+i);
}

void
Parameter::setValue(float value)
{
#ifdef _DEBUG
	float foo = value;
#endif
	float old_value = _value;
	
	if (value<_min)
		value = _min;
	else if(value>_max)
		value = _max;

	if(_step)
		if(value>0)
			_value = _step*(int)((value+(_step/2))/_step);
		else
			_value = _step*(int)((value-(_step/2))/_step);
	else
		_value = value;
	
	// set the control value
	switch (_controlMode) {
		case PARAM_DIRECT:
			_controlValue = _offset + _base * _value;
			break;
		case PARAM_EXP:
			_controlValue = _offset + ::pow( (float)_base, _value );
			break;
		case PARAM_POWER:
			_controlValue = _offset + ::pow( _value, (float)_base );
			break;
#ifdef _DEBUG
		default:
			cout << "<Parameter> mode is undefined" << endl;
			break;
#endif
	}
	
#ifdef _DEBUG
	cout << "<Parameter::setValue( " << foo 
	<< " ) min=" << _min << " max=" << _max << " value set to " << _value 
	<< " _controlValue set to " << _controlValue << endl;
#endif
	
	// TODO: only update() Listeners it there _was_ a change?
	// messes up the GUI - it needs to be better behaved (respect step value)
	if (old_value!=_value)
		for (unsigned i=0; i<_updateListeners.size(); i++)
		{
#ifdef _DEBUG
			cout << "updating UpdateListener " << _updateListeners[i] << endl;
#endif
			_updateListeners[i]->UpdateParameter (mParamId, _controlValue);
		}
}

int
Parameter::getSteps() const
{
	if(!_step)
		return 0;
	
	int i = 0;
	float v = getMin();
	while(v <= getMax()){
		v += getStep();
		i++;
	}
	return i;
}

void
Parameter::random_val()
{
	setValue( ((rand()/(float)RAND_MAX) * (getMax()-getMin()) + getMin()) );
}

void
Parameter::setParameterValueStrings(const char **names, size_t count)
{
	_parameterValueStrings.clear();
	for (size_t i=0; i<count; i++) {
		_parameterValueStrings.push_back(std::string(names[i]));
	}
}
