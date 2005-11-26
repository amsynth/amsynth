/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#ifndef _PARAMETER_H
#define _PARAMETER_H

#include <string>
#include <vector>
#include <sstream>
#include <cmath>
#include "UpdateListener.h"

using std::string;
using std::vector;

#define clip(in,min,max) ((in) < (min) ? (min) : ((in) > (max) ? (max) : (in)))

/**
 * @brief a Parameter holds a particular value for a slider, selector switch 
 * etc..
 *
 * This object is also responsible for keeping all audio/signal generation 
 * objects up to date with any controls which affect them.
 *
 * This object also easily enables non-linear relationships between the controls
 * (eg interface ParameterViews) and their effect on synthesis parameters. See
 * setType() for details.
 */

class Parameter {
public:
	enum ControlType
	{
		PARAM_DIRECT,	// controlValue = offset + base * value
		PARAM_EXP,		// controlValue = offset + base ^ value
		PARAM_POWER		// controlValue = offset + value ^ base
	};

					Parameter		(const string name = "unused", Param id = kControls_End, 
									 float value = 0.0, float min = 0.0, float max = 1.0, float inc = 0.0,
									 ControlType = PARAM_DIRECT, float base = 1.0, float offset = 0.0,
									 const string label = "");

	// The raw value of this parameter. Objects in the signal generation 
	// path should not use this method, but getControlValue() instead.
	float			getValue		() const { return _value; }
	void			setValue		(float value);

	float			GetNormalisedValue	() const { return (getValue()-getMin())/(getMax()-getMin()); }
	void			SetNormalisedValue	(float val) { setValue (val*(getMax()-getMin())+getMin()); }

	// The control value for this parameter.
	// The control value is what the synthesis will use to get its values.
	inline float	getControlValue	() const { return controlValue; }

	const string	GetStringValue	() const { std::ostringstream o; o << controlValue; return o.str(); }

	const string	getName			() const { return _name; }
	Param			GetId			() const { return mParamId; }

	// UpdateListeners (eg one or more ParameterViews - part of the GUI) are 
	// notified and updated when this Parameter changes.
	void			addUpdateListener (UpdateListener& ul);
	void			removeUpdateListener (UpdateListener& ul);
	
	// min/max values apply for calls to setValue() not ControlValue
	float			getMin			() const { return _min; }
	float			getMax			() const { return _max; }


	// @return the increment value
	float			getStep			() const { return _step; }
	// @returns The number of discrete steps allowable in this Parameter.
	int				getSteps		() const;

	// Set this parameter to a random value (in it's allowable range)
	void			random_val		();

	// The label assocaited with this Parameter. (e.g. "seconds")
	const string	getLabel		() const { return label; }
	
private:
	Param							mParamId;
	string							_name, label;
	int								controlMode;
	float							_value, _min, _max, _step, controlValue, base, offset;
	vector<UpdateListener*>			updateListeners;
};

#endif
