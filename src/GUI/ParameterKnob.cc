/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "ParameterKnob.h"
#include <stdio.h>

ParameterKnob::ParameterKnob( int pipe_d )
{
    adj = new Gtk::Adjustment(0.0, 0.0, 1.0, 0.01, 1.0, 0);

    adj->value_changed.
		connect(bind(slot(this, &ParameterKnob::updateParam), adj));
	
	knob.set_adjustment( *adj );
	
    parameter = 0;

    draw_value = false;

    add(label);
    add(knob);
    value_frame.set_shadow_type(GTK_SHADOW_IN);
	value_frame.add( value_label );
	if(draw_value==true)
		add(value_frame);
	
	supress_param_callback = false;
	piped = pipe_d;
	request.slot = slot( *this, &ParameterKnob::_update_ );
}

void
ParameterKnob::setPixmap(GdkPixmap * pix, gint x, gint y, gint frames)
{
	knob.setPixmap( pix, x, y, frames );
}

ParameterKnob::~ParameterKnob()
{
}

Parameter *
ParameterKnob::getParameter()
{
    return parameter;
}

void 
ParameterKnob::drawValue(bool draw)
{
    if (draw_value == true && draw == false)
		remove(	value_frame	);
	if (draw_value == false && draw == true)
		add( value_frame );
    show_all();
		draw_value = draw;
}

void 
ParameterKnob::setParameter(Parameter & param)
{
    parameter = &param;

    label.set_text(parameter->getName());

    adj->set_lower(param.getMin());
    adj->set_upper(param.getMax());
    if (param.getStep())
	adj->set_step_increment(param.getStep());

    param.addUpdateListener(*this);
    update();
}

void 
ParameterKnob::updateParam(Gtk::Adjustment * _adj)
{
	if (!supress_param_callback)
    parameter->setValue(_adj->get_value());
}

void 
ParameterKnob::setName(string text)
{
    label.set_text(text);
}

void
ParameterKnob::update()
{
	if(!supress_param_callback)
		if( write( piped, &request, sizeof(request) ) != sizeof(request) )
			cout << "error writing to pipe" << endl;
}

void 
ParameterKnob::_update_()
{
	supress_param_callback = true;
	
    adj->set_value(parameter->getValue());
	
    char cstr[6];
    sprintf(cstr, "%.3f", parameter->getControlValue());
    string text(cstr);
    text += " ";
    text += parameter->getLabel();
	
    value_label.set_text(text);
	
	supress_param_callback = false;
}

Gtk::Widget *
ParameterKnob::getGtkWidget()
{
    return (Gtk::Widget *) & knob;
}

void 
ParameterKnob::set_style( Gtk::Style& style )
{
	value_label.set_style( style );
	label.set_style( style );
}
