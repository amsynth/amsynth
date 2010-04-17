/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#ifndef _PARAMETERSWITCH_H
#define _PARAMETERSWITCH_H

#include <gtkmm.h>
#include <stdlib.h>

#include "ParameterView.h"

class ParameterSwitch : public ParameterView, public Gtk::VBox {
  public:
    ParameterSwitch();

    void setParameter(Parameter & param);
    void setName(string name);
	void _update_();
	void toggle_handler();
  private:
	Gtk::CheckButton check_button;
	Gtk::Label label;
	gboolean supress_param_callback;
};

#endif
