/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#include "Parameter.h"

#ifdef _DEBUG
#include <iostream>
#endif

Parameter::Parameter	(string name, Param id, float value, float min, float max, float inc, ControlType type, float base, float offset, string label)
:	mParamId	(id)
,	_name		(name)
,	label		(label)
,	controlMode	(type)
,	continuous	(0.0f < _step)
,	_min		(min)
,	_max		(max)
,	_step		(inc)
,	controlValue(0)
,	base		(base)
,	offset		(offset)
{
	setValue (value);
}

void
Parameter::addUpdateListener	(UpdateListener& ul)
{
	for (unsigned i=0; i<updateListeners.size(); i++) if (updateListeners[i] == &ul) return;
	updateListeners.push_back (&ul);
	updateListeners.back()->UpdateParameter (mParamId, controlValue);
}

void
Parameter::removeUpdateListener( UpdateListener & ul )
{
	for (unsigned i=0; i<updateListeners.size(); i++)
		if (updateListeners[i] == &ul) updateListeners.erase(updateListeners.begin()+i);
}

void
Parameter::setType	(ControlType type, float base, float offset)
{	
	controlMode = type;
	this->base = base;
	this->offset = offset;
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
	switch(controlMode){
		case PARAM_DIRECT:
			controlValue = offset + base*_value;
		break;
		case PARAM_EXP:
			controlValue = offset + ::pow((float)base,_value);
		break;
		case PARAM_POWER:
			controlValue = offset + ::pow( _value, (float)base );
#ifdef _DEBUG
		default:
		cout << "<Parameter> mode is undefined" << endl;
		break;
#endif
	}
	
#ifdef _DEBUG
	cout << "<Parameter::setValue( " << foo 
	<< " ) min=" << _min << " max=" << _max << " value set to " << _value 
	<< " controlValue set to " << controlValue << endl;
#endif
	
	// TODO: only update() Listeners it there _was_ a change?
	// messes up the GUI - it needs to be better behaved (respect step value)
	if (old_value!=_value)
		for (unsigned i=0; i<updateListeners.size(); i++)
		{
#ifdef _DEBUG
			cout << "updating UpdateListener " << updateListeners[i] << endl;
#endif
			updateListeners[i]->UpdateParameter (mParamId, controlValue);
		}
}

int
Parameter::getSteps()
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
	setValue( ((rand()/(double)RAND_MAX) * (getMax()-getMin()) + getMin()) );
}
