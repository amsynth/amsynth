/*
 *  Parameter.h
 *
 *  Copyright (c) 2001-2012 Nick Dowell
 *
 *  This file is part of amsynth.
 *
 *  amsynth is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  amsynth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with amsynth.  If not, see <http://www.gnu.org/licenses/>.
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
,	_valueStrings(NULL)
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
