/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "RadioButtonParameterView.h"
#include <stdio.h>

using SigC::slot;
using SigC::bind;

RadioButtonParameterView::RadioButtonParameterView( int pipe_d )
{
	last_toggle = 0.0;
	parameter = 0;

	add( frame );
	frame.add( vbox );
	for(gint i=1; i<MAX_BUTTONS; i++)
		radio_button[i].set_group( radio_button[0].group() );
	
	supress_param_callback = false;
	request.slot = slot( *this, &RadioButtonParameterView::_update_ );
	piped = pipe_d;
}

RadioButtonParameterView::~RadioButtonParameterView()
{
	if(parameter)
		parameter->removeUpdateListener(*this);
}

void
RadioButtonParameterView::update()
{
	if(!supress_param_callback)
		if( write( piped, &request, sizeof(request) ) != sizeof(request) )
			cout << "error writing to pipe" << endl;
}

void
RadioButtonParameterView::_update_()
{
	supress_param_callback = true;
	gdk_threads_enter();
	if(parameter){
		gint button = (gint)( (parameter->getMax()-parameter->getValue())/parameter->getStep() );
		if(button>=0 && button<MAX_BUTTONS && !radio_button[button].get_active() )
			radio_button[button].set_active( true );
	}
	gdk_threads_leave();
	supress_param_callback = false;
}

void
RadioButtonParameterView::setParameter( Parameter & param ){
	parameter = &param;
	parameter->addUpdateListener( *this );
	
	frame.set_label( parameter->getName() );
	
	gchar btxt[20];
	gfloat v = parameter->getMax();
	for(gint i=0; i<parameter->getSteps(); i++){
		sprintf( btxt, "%f", v );
		radio_button[i].add_label( string(btxt) );
		button_value[i] = v;
		radio_button[i].toggled.connect(bind(slot(this, &RadioButtonParameterView::toggle_handler), i));
		v -= parameter->getStep();
		vbox.add( radio_button[i] );
	}
	show_all(); // otherwise the add()ed widgets arent dispayed...
	update();
}

Parameter *
RadioButtonParameterView::getParameter()
{
	return parameter;
}

void
RadioButtonParameterView::toggle_handler( gint button )
{
	if(!supress_param_callback){
		if (button!=last_toggle && parameter){
			last_toggle = button;
			if (parameter->getValue()!=(float)button_value[button])
				parameter->setValue( (float)button_value[button] );
		}
	}
}

void 
RadioButtonParameterView::setName( string name )
{
	frame.set_label( name );
}

void 
RadioButtonParameterView::setDescription( int button, string text )
{
	radio_button[button].remove();
	radio_button[button].add_label( text );
}
