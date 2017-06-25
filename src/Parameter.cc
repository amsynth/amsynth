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

#include <algorithm>
#include <cassert>

Parameter::Parameter	(std::string name, Param id, float value, float min, float max, float inc, ControlType type, float base, float offset, std::string label)
:	mParamId	(id)
,	_name		(name)
,	_label		(label)
,	_controlMode	(type)
,	_default	(value)
,	_value		(NAN)
,	_min		(min)
,	_max		(max)
,	_step		(inc)
,	_controlValue(NAN)
,	_base		(base)
,	_offset		(offset)
{
	assert(min < max);
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
	float newValue = std::min(std::max(value, _min), _max);

	if (_step) {
		newValue = _min + roundf((newValue - _min) / _step) * _step;
		assert(::fmodf(newValue - _min, _step) == 0);
	}

	if (_value == newValue) // warning: -ffast-math causes this comparison to fail
		return;

	_value = newValue;

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
	}

	for (unsigned i=0; i<_updateListeners.size(); i++) {
		_updateListeners[i]->UpdateParameter (mParamId, _controlValue);
	}
}

void
Parameter::random_val()
{
	setValue( ((rand()/(float)RAND_MAX) * (getMax()-getMin()) + getMin()) );
}
