/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "Parameter.h"

Parameter::Parameter()
{
	for (int i=0; i<MAX_ULS; i++) updateListeners[i] = 0;
    _value = 0.0;
    _name = "unused";
    _min = 0.0;
    _max = 1.0;
    _step = 0;
	continuous = true;
	controlMode = 1;
	base = 1;
	offset = 0;
}

Parameter::~Parameter()
{
#ifdef _DEBUG
	cout << "<Parameter::~Parameter()>" << endl;
#endif
}

void
Parameter::addUpdateListener( UpdateListener & ul )
{
	for (int i=0; i<MAX_ULS; i++)
		if (&ul == updateListeners[i])
			return;
	
	for (int i=1; i<MAX_ULS; i++)
		if (!updateListeners[i]) {
			updateListeners[i] = &ul;
			return;
		}
}

void
Parameter::removeUpdateListener( UpdateListener & ul )
{
	for (int i=0; i<MAX_ULS; i++)
		if (updateListeners[i] == &ul) {
			updateListeners[i] = 0;
			return;
		}
}

void
Parameter::setType( int type, float base, float offset )
{	controlMode = type;
	this->base = base;
	this->offset = offset;
}

void
Parameter::setValue(float value)
{
	float foo = value;
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
			controlValue = offset + pow((double)base,_value);
		break;
		case PARAM_POWER:
			controlValue = offset + pow( _value, (double)base );
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
//	if (old_value!=_value)
		for (int i=0; i<MAX_ULS; i++)
			if (updateListeners[i]){
#ifdef _DEBUG
				cout << "updating UpdateListener " << updateListeners[i] << endl;
#endif
				updateListeners[i]->update();
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

float
Parameter::getValue()
{
	return _value;
}

void
Parameter::setLabel( string text )
{
	label = text;
}

string
Parameter::getLabel()
{
	return label;
}

void
Parameter::random_val()
{
	setValue( ((random()/(double)RAND_MAX) * (getMax()-getMin()) + getMin()) );
}