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
    ParameterSwitch();
    ~ParameterSwitch();
    void setParameter(Parameter & param);
    Parameter *getParameter();
    void setName(string name);
    void update();
	void toggle_handler();
  private:
	Gtk::CheckButton check_button;
	Gtk::Label label;
	Parameter *parameter;
	int paramName;
};

#endif
