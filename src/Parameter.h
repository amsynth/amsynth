/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

/**
 * a Parameter holds a particular value for a slider, selector switch etc..
 */

#ifndef _PARAMETER_H
#define _PARAMETER_H

#include <string>
#include <math.h>
#include "UpdateListener.h"

#define MAX_ULS 256
#define PARAM_DIRECT 1
#define PARAM_EXP 2
#define PARAM_POWER 3

class Parameter {
public:
  Parameter();
  ~Parameter();
  /**
   * returns the value of this parameter
   **/
  float getValue();
  /**
   * returns the control value for this parameter.
   * the control value is what the synthesis will use to get its values.
   * in most cases it is the same as getValue(), but some controls may use a 
   * logarithmic or exponential scale, for which this function returns the 
   * appropriate value
   **/
  inline float getControlValue()
  { return controlValue; };
  /**
   * set a new value for this parameter.
   * doing this will automatically cause the ParameterView (part of the GUI)
   * as given by setParameterView() to be updated etc..
   **/
  void setValue(float value);
  /**
   * get the name given to this parameter. Useful for identifying Parameters.
   **/
  string getName()
  { return _name; };
  /**
   * give this Parameter a name.
   * @param name the name to identify this parameter
   **/
  void setName(string name)
  { _name = name; };
  /**
   * set the mode for this parameter.
   * the Parameter can be direct, type : PARAM_DIRECT
   * it can be exponential : PARAM_EXP where the control value will be base^value
   **/
  void setType( int type, float base, float offset );
  /**
   *set the ParameterView to be associated with this Parameter. this allows
   * the ParameterView (part of the GUI) to be updated when this Parameter
   * changes.
   * @param p_v the ParameterView to be associated with this Parameter.
   **/
	void setUpdateListener( UpdateListener & ul )
	{ updateListeners[0] = &ul; };
	
	void addUpdateListener( UpdateListener & ul );
	void removeUpdateListener( UpdateListener & ul );
	
	void setMin( float min )
	{ _min = min; };

	float getMin()
	{ return _min; };
	
	void setMax( float max )
	{ _max = max; };
	
	float getMax()
	{ return _max; };
	
	void setStep( float step )
	{ _step = step; };
	
	int getSteps();
	
	float getStep()
	{ return _step; };
	
	bool isContinuous()
	{ return continuous; };
	
	void setContinuous( bool continuous )
	{ this->continuous = continuous; };
	
	bool isDiscrete()
	{ return !continuous; }
	
	void setDiscrete( bool discrete )
	{ continuous = !discrete; };
	
	void random_val();
	
	string getLabel();
	void setLabel( string text );
	
private:
	string _name, label;
	int controlMode;
	bool continuous;
	float _value, _min, _max, _step, controlValue, base, offset;
	UpdateListener *updateListeners[MAX_ULS];
};

#endif
