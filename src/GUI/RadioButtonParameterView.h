/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#ifndef _RADIOBUTTONPARAMETERVIEW_H
#define _RADIOBUTTONPARAMETERVIEW_H

#include <gtkmm/box.h>
#include <gtkmm/frame.h>
#include <gtkmm/label.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/style.h>
#include <stdlib.h>
#include <list>
#include <string>
#include "ParameterView.h"

#define MAX_BUTTONS 10 // anything more than that would be silly!

class RadioButtonParameterView : public ParameterView, public Gtk::VBox {
public:
	RadioButtonParameterView( int pipe_d );
	void setName( string name );
	void setDescription( int button, string text );
	void setParameter( Parameter & param );
	void _update_();
private:
	void toggle_handler( int button );
	gfloat last_toggle;
	gboolean supress_param_callback;
	Gtk::Label label;
	Gtk::Frame frame;
	Gtk::VBox vbox;
	Gtk::RadioButton radio_button[MAX_BUTTONS];
	Gtk::Style *local_style;
	gfloat button_value[MAX_BUTTONS]; // holds parameter value attatched to a radio button
};

#endif
