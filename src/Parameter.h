/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#ifndef _PARAMETER_H
#define _PARAMETER_H

#include <string>
#include <math.h>
#include "base.h"
#include "UpdateListener.h"

#define MAX_ULS 256
#define PARAM_DIRECT 1
#define PARAM_EXP 2
#define PARAM_POWER 3

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
	Parameter();
	~Parameter();
	/**
	 * Returns the value of this parameter. Objects in the signal generation 
	 * path should not use this method, but getControlValue() instead.
	**/
	float getValue();
	/**
	 * Returns the control value for this parameter.
	 * The control value is what the synthesis will use to get its values.
	 * in most cases it is the same as getValue(), but some controls may use a 
	 * logarithmic or exponential scale, for which this function returns the 
	 * appropriate value
	**/
	inline float getControlValue()
	{ return controlValue; };
	/**
	 * Set a new value for this parameter.
	 * Doing this will automatically cause the ParameterView (part of the GUI)
	 * as given by setParameterView() to be updated etc..
	**/
	void setValue(float value);
	/**
	 * Get the name given to this parameter. Useful for identifying Parameters.
	**/
	string getName()
	{ return _name; };
	/**
	 * Give this Parameter a name.
	 * @param name the name to identify this parameter
	**/
	void setName(string name)
	{ _name = name; };
	/**
	 * Set the mode for this parameter.
	 * The Parameter can be direct, type : PARAM_DIRECT
	 * it can be exponential : PARAM_EXP where the control value will be 
	 * base^value
	**/
	void setType( int type, float base, float offset );
	/**
	 * Add an UpdateListener to be associated with this Parameter. This allows
	 * the UpdateListener (eg one or more ParameterViews - part of the GUI) to 
	 * be notified and updated when this Parameter changes.
	 * @param ul An UpdateListener to be associated with this Parameter.
	**/	
	void addUpdateListener( UpdateListener & ul );
	/**
	 * Remove an UpdateListener from being associated with this Parameter.
	 * @param ul The UpdateListener to no longer be associated with this 
	 * Parameter.
	**/	
	void removeUpdateListener( UpdateListener & ul );
	
	void setUpdateListener( UpdateListener & ul )
	{ updateListeners[0] = &ul; };
	/**
	 * @param The minimum value (e.g. for calls to setValue(), NOT 
	 * getControlValue()) allowed for this Parameter.
	 */
	void setMin( float min )
	{ _min = min; if(_value<_min)_value=_min; };
	/**
	 * @returns The minimum value (e.g. for calls to setValue(), NOT 
	 * getControlValue()) allowed for this Parameter.
	 */
	float getMin()
	{ return _min; };
	/**
	 * @param The maximum value (e.g. for calls to setValue(), NOT 
	 * getControlValue()) allowed for this Parameter.
	 */
	void setMax( float max )
	{ _max = max; if(_value>_max)_value=_max; };
	/**
	 * @returns The maximum value (e.g. for calls to setValue(), NOT 
	 * getControlValue()) allowed for this Parameter.
	 */
	float getMax()
	{ return _max; };
	/**
	 * @param The minimum increment in value (e.g. for calls to setValue(), NOT 
	 * getControlValue()) allowed for this Parameter.
	 */
	void setStep( float step )
	{ _step = step; };
	/**
	 * @returns The number of discrete steps allowable in this Parameter.
	 */
	int getSteps();
	/**
	 * @returns The minimum increment in value (e.g. for calls to setValue(), 
	 * NOT getControlValue()) allowed for this Parameter.
	 */
	float getStep()
	{ return _step; };
	/**
	 * @returns true if this Parameter's value is continuous (eg step=0).
	 */
	bool isContinuous()
	{ return continuous; };
	/**
	 * Set whether this Parameter's value is continous (eg step=0).
	 */
	void setContinuous( bool continuous )
	{ this->continuous = continuous; };
	/**
	 * @returns true if this Parameter's value is discrete.
	 */
	bool isDiscrete()
	{ return !continuous; }
	/**
	 * Set whether this Parameter's value is discrete.
	 */
	void setDiscrete( bool discrete )
	{ continuous = !discrete; };
	/**
	 * Set this parameter to a random value (in it's allowable range)
	 */
	void random_val();
	/**
	 * @returns The label assocaited with this Parameter. (e.g. "seconds")
	 */
	string getLabel();
	/**
	 * @param text The label to be assocaited with this Parameter. 
	 * (e.g. "seconds")
	 */
	void setLabel( string text );
	
private:
	string _name, label;
	int controlMode;
	bool continuous;
	float _value, _min, _max, _step, controlValue, base, offset;
	UpdateListener *updateListeners[MAX_ULS];
};

#endif
