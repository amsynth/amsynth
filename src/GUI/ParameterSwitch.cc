/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#include "ParameterSwitch.h"
#include <iostream>

ParameterSwitch::ParameterSwitch()
:	supress_param_callback (false)
{
	check_button.add (label);
	add (check_button);
	
	check_button.signal_toggled().connect (mem_fun (*this, &ParameterSwitch::toggle_handler));
}

void
ParameterSwitch::toggle_handler()
{
	if (supress_param_callback) return;
	if (!parameter) return;
	
	supress_param_callback = true;
	if (check_button.get_active())	parameter->setValue (parameter->getMax());
	else							parameter->setValue (parameter->getMin());
	supress_param_callback = false;
}

void
ParameterSwitch::setParameter( Parameter & param )
{
//	if (2 != param.getSteps ()) return; // only makes sense if the Parameter has 2 possible vals

	ParameterView::setParameter (param);
	label.set_text (parameter->getName());
//	parameter = &param;
//	parameter->addUpdateListener( *this );
//	update();
}

void
ParameterSwitch::setName( string name )
{
	label.set_text( name );
}

void
ParameterSwitch::_update_()
{
	if (supress_param_callback) return;
	if (!parameter) return;
	
	supress_param_callback = true;
	check_button.set_active (parameter->GetNormalisedValue() > 0.5);
	supress_param_callback = false;
}

