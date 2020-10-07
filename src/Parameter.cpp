/*
 *  Parameter.h
 *
 *  Copyright (c) 2001-2020 Nick Dowell
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

#include "VoiceBoard/Synth--.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <sstream>

Parameter::Parameter(const std::string &name, Param id, float value, float min, float max, float inc, Law law, float base, float offset, const std::string &label)
:	_paramId	(id)
,	_name		(name)
,	_label		(label)
,	_law		(law)
,	_default	(value)
,	_value		(m::nan)
,	_min		(min)
,	_max		(max)
,	_step		(inc)
,	_controlValue(m::nan)
,	_base		(base)
,	_offset		(offset)
{
	assert(min < max);
	setValue (value);
}

void
Parameter::addUpdateListener(UpdateListener *listener)
{
	_listeners.insert(listener);
	listener->UpdateParameter(_paramId, _controlValue);
}

void
Parameter::removeUpdateListener(UpdateListener *listener)
{
	_listeners.erase(listener);
}

void
Parameter::setValue(float value)
{
	float newValue = std::min(std::max(value, _min), _max);

	if (_step > 0.f) {
		newValue = _min + roundf((newValue - _min) / _step) * _step;
		assert(::fmodf(newValue - _min, _step) == 0);
	}

	if (_value == newValue) // warning: -ffast-math causes this comparison to fail
		return;

	_value = newValue;

	switch (_law) {
		case Law::kLinear:
			_controlValue = _offset + _base * _value;
			break;
		case Law::kExponential:
			_controlValue = _offset + ::pow( (float)_base, _value );
			break;
		case Law::kPower:
			_controlValue = _offset + ::pow( _value, (float)_base );
			break;
	}

	for (auto &listener : _listeners) {
		listener->UpdateParameter(_paramId, _controlValue);
	}
}

float
Parameter::valueFromString(const std::string &str)
{
	// atof() and friends are affected by currently configured locale,
	// which can change the decimal point character.
	std::istringstream istr(str);
	static std::locale locale = std::locale("C");
	istr.imbue(locale);
	float value = m::nan;
	istr >> value;
	return value;
}

const std::string
Parameter::getStringValue() const
{
	std::ostringstream stream;
	stream << _controlValue;
	return stream.str();
}

void
Parameter::randomise()
{
	setValue( ((rand()/(float)RAND_MAX) * (getMax()-getMin()) + getMin()) );
}
