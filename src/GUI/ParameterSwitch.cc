/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "ParameterSwitch.h"

ParameterSwitch::ParameterSwitch()
{
	paramName = 1;
	
	check_button.add( label );
	add( check_button );
	
	check_button.toggled.connect( 
		slot(this, &ParameterSwitch::toggle_handler) );
}

ParameterSwitch::~ParameterSwitch()
{
}

void
ParameterSwitch::toggle_handler()
{
	if(check_button.get_active()==true)
		parameter->setValue( parameter->getMax() );
	else
		parameter->setValue( parameter->getMin() );
}

void
ParameterSwitch::setParameter( Parameter & param )
{
	paramName = 1;
	if(param.getSteps()==2){ 
		// only makes sense if the Parameter has 2 possible vals
		parameter = &param;
		parameter->addUpdateListener( *this );
		update();
	}
}

Parameter *
ParameterSwitch::getParameter()
{	return parameter;
}

void
ParameterSwitch::setName( string name )
{
	paramName = 0;
	label.set_text( name );
}

void
ParameterSwitch::update()
{
	if( paramName ) 
		label.set_text( parameter->getName() );
	if( parameter && (parameter->getValue() == parameter->getMax()) ) 
		check_button.set_active( true );
	else
		check_button.set_active( false );
}
