/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#include "RadioButtonParameterView.h"
#include <stdio.h>
#include <iostream>

using sigc::bind;

RadioButtonParameterView::RadioButtonParameterView( int pipe_d )
:	ParameterView (pipe_d)
{
	last_toggle = 0.0;
	local_style = 0;

	add( frame );
	frame.add( vbox );
	Gtk::RadioButtonGroup group = radio_button[0].get_group();
	for(gint i=1; i<MAX_BUTTONS; i++) radio_button[i].set_group (group);
	
	supress_param_callback = false;
}

void
RadioButtonParameterView::_update_()
{
	if (!supress_param_callback)
	{
		supress_param_callback = true;
//		gdk_threads_enter();
		if(parameter){
			gint button = (gint)( (parameter->getMax()-parameter->getValue())/parameter->getStep() );
			if(button>=0 && button<MAX_BUTTONS && !radio_button[button].get_active() )
				radio_button[button].set_active( true );
		}
//		gdk_threads_leave();
		supress_param_callback = false;
	}
}

void
RadioButtonParameterView::setParameter( Parameter & param )
{
	frame.set_label( param.getName() );
	
	gchar btxt[20];
	gfloat v = param.getMax();
	for(gint i=0; i<param.getSteps(); i++){
		sprintf( btxt, "%f", v );
		radio_button[i].add_label( string(btxt) );
		button_value[i] = v;
		radio_button[i].signal_toggled().connect(bind(mem_fun(*this, &RadioButtonParameterView::toggle_handler), i));
		v -= param.getStep();
		vbox.add( radio_button[i] );
	}
	show_all(); // otherwise the add()ed widgets arent dispayed...
	
	ParameterView::setParameter (param);	
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
//	if( local_style!= 0 )
//		radio_button[button].get_child()->set_style( *local_style );
}

void 
RadioButtonParameterView::set_style( Gtk::Style& style )
{
//	local_style = style.copy();
//	frame.set_style( *local_style );
//	for (int i=0; i<MAX_BUTTONS; i++)
//		if( radio_button[i].get_child() )
//			radio_button[i].get_child()->set_style( *local_style );
}
