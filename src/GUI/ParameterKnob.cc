/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "ParameterKnob.h"
#include <gtkmm/alignment.h>
#include <stdio.h>
#include <iostream>
#include <iomanip>
#include <sstream>

using namespace std;

ParameterKnob::ParameterKnob( int pipe_d )
  : ParameterView (pipe_d), draw_value(false)
{
  adj = new Gtk::Adjustment(0.0, 0.0, 1.0, 0.01, 1.0, 0);
  
  adj->signal_value_changed().
    connect(sigc::bind(mem_fun(*this, &ParameterKnob::updateParam), adj));
  knob.set_adjustment(adj);
	
  label.set_justify(Gtk::JUSTIFY_CENTER);
  pack_start(label,0,0);
  Gtk::Alignment *align = manage(new Gtk::Alignment(Gtk::ALIGN_CENTER,
						    Gtk::ALIGN_CENTER,
						    0,0));
  align->add(knob);
  pack_start(*align,0,0);
  value_frame.add( value_label );
  if(draw_value)
    pack_start(value_frame,0,0);
	
  supress_param_callback = false;
}

void
ParameterKnob::setFrames(const Glib::RefPtr<Gdk::Pixbuf>& src, int x, int y, int numFrames)
{
	knob.setFrames(src, x,y, numFrames);
}

void 
ParameterKnob::drawValue(bool draw)
{
  if(draw != draw_value) {
    if(draw)
      pack_start(value_frame,0,0);
    else
      remove(value_frame);
  }
  draw_value = draw;
}

void 
ParameterKnob::setParameter(Parameter & param)
{
  label.set_text(param.getName());

  adj->set_lower(param.getMin());
  adj->set_upper(param.getMax());
  if (param.getStep()) adj->set_step_increment(param.getStep());

  ParameterView::setParameter (param);
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
ParameterKnob::_update_()
{
  if (!supress_param_callback)
    {
      supress_param_callback = true;
		
      adj->set_value(parameter->getValue());

      ostringstream text;
      text << setiosflags(ios_base::fixed);
      double value = parameter->getControlValue();
      if(value < 10)
	   text << setprecision(4);
      else
	text << setprecision(3);
      text << value << " "
	   << parameter->getLabel();
      value_label.set_text(text.str());
		
      supress_param_callback = false;
    }
}

Gtk::Widget *
ParameterKnob::getGtkWidget()
{
  return (Gtk::Widget *) & knob;
}

void 
ParameterKnob::set_style( Gtk::Style& style )
{
  //	value_label.set_style( style );
  //	label.set_style( style );
}
