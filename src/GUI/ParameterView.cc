/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "ParameterView.h"
#include <stdio.h>

ParameterView::ParameterView()
{
    adj = new Gtk::Adjustment(0.0, 0.0, 1.0, 0.01, 1.0, 0);

    adj->value_changed.
		connect(bind(slot(this, &ParameterView::updateParam), adj));
	
	knob.set_adjustment( *adj );
	
    parameter = 0;

    draw_value = false;

    add(label);
    add(knob);
    value_frame.set_shadow_type(GTK_SHADOW_IN);
	value_frame.add( value_label );
	if(draw_value==true)
		add(value_frame);
}

void
ParameterView::setPixmap(GdkPixmap * pix, gint x, gint y, gint frames)
{
	knob.setPixmap( pix, x, y, frames );
}

ParameterView::~ParameterView()
{
}

Parameter *
ParameterView::getParameter()
{
    return parameter;
}

void 
ParameterView::drawValue(bool draw)
{
    if (draw_value == true && draw == false)
		remove(	value_frame	);
	if (draw_value == false && draw == true)
		add( value_frame );
    show_all();	draw_value = draw;
}

void 
ParameterView::setParameter(Parameter & param)
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
ParameterView::updateParam(Gtk::Adjustment * _adj)
{
    parameter->setValue(_adj->get_value());
}

void 
ParameterView::setName(string text)
{
    label.set_text(text);
}

void 
ParameterView::update()
{
    adj->set_value(parameter->getValue());
    char cstr[20];
    sprintf(cstr, "%.2f", parameter->getControlValue());
    string text(cstr);
    text += " ";
    text += parameter->getLabel();
    value_label.set_text(text);
}

Gtk::Widget * 
ParameterView::getGtkWidget()
{
    return (Gtk::Widget *) & knob;
}
