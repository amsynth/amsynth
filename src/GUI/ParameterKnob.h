/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#ifndef _PARAMETERKNOB_H
#define _PARAMETERKNOB_H

#include <gtkmm/box.h>
#include <gtkmm/menu.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/adjustment.h>
#include <stdlib.h>
#include "ParameterView.h"
#include "Knob.h"


class ParameterKnob : public ParameterView, public Gtk::VBox
{
public:
	ParameterKnob( int pipe_d );
 
	void setParameter(Parameter & param);
    void setName(string name);
	void _update_();
    Gtk::Widget * getGtkWidget();
	void drawValue( bool draw );
	void setPixmap( GdkPixmap *pix, gint x, gint y, gint frames );
	void set_style( Gtk::Style& style );
	
private:
    void updateParam(Gtk::Adjustment * _adj);
    Knob knob;
	gboolean draw_value, supress_param_callback;
    Gtk::Label label, value_label;
    Gtk::Adjustment * adj;
	Gtk::Frame value_frame;
};


#endif
