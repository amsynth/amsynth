/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#ifndef _PARAMETERSWITCH_H
#define _PARAMETERSWITCH_H

#include <gtk--/box.h>
#include <gtk--/label.h>
#include <gtk--/checkbutton.h>
#include <stdlib.h>

#include "ParameterView.h"

class ParameterSwitch : public ParameterView, public Gtk::VBox {
  public:
    ParameterSwitch( int pipe_d );
    ~ParameterSwitch();
    void setParameter(Parameter & param);
    Parameter *getParameter();
    void setName(string name);
    void update();
	void _update_();
	void toggle_handler();
	void set_style( Gtk::Style& style );
  private:
	Gtk::CheckButton check_button;
	Gtk::Label label;
	Parameter *parameter;
	int paramName;
	gboolean supress_param_callback;
};

#endif
