/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#ifndef _PARAMETER_H
#define _PARAMETER_H

#include <string>
#include <vector>
#include <sstream>
#include <cmath>
#include "base.h"
#include "UpdateListener.h"

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

					Parameter		(string name = "unused", Param id = kControls_End, 
									 float value = 0.0, float min = 0.0, float max = 1.0, float inc = 0.0,
									 ControlType = PARAM_DIRECT, float base = 1.0, float offset = 0.0,
									 string label = "");

	// The raw value of this parameter. Objects in the signal generation 
	// path should not use this method, but getControlValue() instead.
	float			getValue		() { return _value; }
	void			setValue		(float value);

	float			GetNormalisedValue	() { return (getValue()-getMin())/(getMax()-getMin()); }
	void			SetNormalisedValue	(float val) { setValue (val*(getMax()-getMin())+getMin()); }

	// The control value for this parameter.
	// The control value is what the synthesis will use to get its values.
	inline float	getControlValue	() { return controlValue; }

	const string	GetStringValue	() { std::ostringstream o; o << controlValue; return o.str(); }

	const string	getName			() { return _name; }
	Param			GetId			() { return mParamId; }

	// UpdateListeners (eg one or more ParameterViews - part of the GUI) are 
	// notified and updated when this Parameter changes.
	void			addUpdateListener (UpdateListener& ul);
	void			removeUpdateListener (UpdateListener& ul);
	
	// min/max values apply for calls to setValue() not ControlValue
	float			getMin			() { return _min; }
	float			getMax			() { return _max; }


	// @return the increment value
	float			getStep			() { return _step; }
	// @returns The number of discrete steps allowable in this Parameter.
	int				getSteps		();

	// Set this parameter to a random value (in it's allowable range)
	void			random_val		();

	// The label assocaited with this Parameter. (e.g. "seconds")
	const string	getLabel		() { return label; }
	
private:
	Param							mParamId;
	string							_name, label;
	int								controlMode;
	float							_value, _min, _max, _step, controlValue, base, offset;
	std::vector<UpdateListener*>	updateListeners;
};

class mParameter
{
public:
			mParameter (string name, float min=0.0, float max=1.0, float def=0.0, string label = "") 
				: mName(name), mLabel(label), mMin(min), mMax(max) { SetNativeValue (def); }

	string	GetName				() { return mName; }
	string	GetLabel			() { return mLabel; }

	string	GetStringValue		() { std::ostringstream o; o << mNativeValue; return o.str(); }

	void	SetNormalisedValue	(float val)
	{ 
		mNormalisedValue = clip(val,0.0,1.0);
		mNativeValue = (mMax-mMin) * mNormalisedValue + mMin;
	}
	void	SetNativeValue		(float val)
	{
		mNativeValue = clip(val,mMin,mMax);
		mNormalisedValue = (val-mMin)/(mMax-mMin);
	}
	float	GetNormalisedValue	() { return mNormalisedValue; }
	float	GetNativeValue		() { return mNativeValue; }

private:
	string		mName;
	string		mLabel;
	float		mNormalisedValue;
	float		mNativeValue;
	float		mMin;
	float		mMax;
};

#endif
