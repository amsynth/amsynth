/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "ParameterKnob.h"
#include <stdio.h>

ParameterKnob::ParameterKnob()
{
    adj = new Gtk::Adjustment(0.0, 0.0, 1.0, 0.01, 1.0, 0);

    adj->value_changed.
		connect(bind(slot(this, &ParameterKnob::updateParam), adj));
	
	knob.set_adjustment( *adj );
	
    parameter = 0;

//    set_homogeneous(false);
    draw_value = false;

    add(label);
    add(knob);
    value_frame.set_shadow_type(GTK_SHADOW_IN);
	value_frame.add( value_label );
	if(draw_value==true)
		add(value_frame);
}

void
ParameterKnob::setPixmap(GdkPixmap * pix, gint x, gint y, gint frames)
{
	knob.setPixmap( pix, x, y, frames );
	/*
    cout << "ParameterView::setPixmap" << endl;
    knob = new Knob(pix, x, y, frames);
    cout << ":O" << endl;
//      knob->set_adjustment( *adj );
    cout << ":0" << endl;
    add(*knob);
    show_all();
	*/
}

ParameterKnob::~ParameterKnob()
{
//      delete adj;
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
	
//	set_usize( 	width(), 
//				height()+value_frame.height() );	draw_value = draw;
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
//	gdk_threads_enter();
    parameter->setValue(_adj->get_value());
//	gdk_threads_leave();
}

void 
ParameterKnob::setName(string text)
{
    label.set_text(text);
}

void 
ParameterKnob::update()
{
    adj->set_value(parameter->getValue());
    char cstr[6];
    sprintf(cstr, "%.3f", parameter->getControlValue());
    string text(cstr);
    text += " ";
    text += parameter->getLabel();
    value_label.set_text(text);
}

Gtk::Widget *
ParameterKnob::getGtkWidget()
{
    return (Gtk::Widget *) & knob;
}
