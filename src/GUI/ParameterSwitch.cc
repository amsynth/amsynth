/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "ParameterSwitch.h"
#include <iostream>

ParameterSwitch::ParameterSwitch( int pipe_d )
{
	paramName = 1;
	
	check_button.add( label );
	add( check_button );
	
	check_button.toggled.connect( 
		slot(this, &ParameterSwitch::toggle_handler) );
	
	supress_param_callback = false;
	piped = pipe_d;
	request.slot = slot( this, &ParameterSwitch::_update_ );
}

ParameterSwitch::~ParameterSwitch()
{
}

void
ParameterSwitch::toggle_handler()
{
	if(!supress_param_callback){
		if(check_button.get_active()==true)
			parameter->setValue( parameter->getMax() );
		else
			parameter->setValue( parameter->getMin() );
	}
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
	if(!supress_param_callback)
		if( write( piped, &request, sizeof(request) ) != sizeof(request) )
			cout << "ParameterSwitch: error writing to pipe" << endl;
}

void
ParameterSwitch::_update_()
{
	supress_param_callback = true;
	
	if( paramName )	label.set_text( parameter->getName() );
	if( parameter && (parameter->getValue() == parameter->getMax()) ) check_button.set_active( true );
	else check_button.set_active( false );
	
	supress_param_callback = false;
}

void 
ParameterSwitch::set_style( Gtk::Style& style )
{
	label.set_style( style );
	check_button.get_child()->set_style( style );
}
