/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#ifndef _PARAMETERKNOB_H
#define _PARAMETERKNOB_H

#include <gtk--/box.h>
#include <gtk--/frame.h>
#include <gtk--/label.h>
#include <gtk--/adjustment.h>
#include <stdlib.h>
#include "ParameterView.h"
#include "Knob.h"

class ParameterKnob : public ParameterView, public Gtk::VBox {
  public:
    ParameterKnob();
    ~ParameterKnob();
    void setParameter(Parameter & param);
    Parameter *getParameter();
    void setName(string name);
    void update();
    Gtk::Widget * getGtkWidget();
	void drawValue( bool draw );
	void setPixmap( GdkPixmap *pix, gint x, gint y, gint frames );
  private:
    void updateParam(Gtk::Adjustment * _adj);
    Parameter *parameter;
    Knob knob;
	bool draw_value;
    Gtk::Label label, value_label;
    Gtk::Adjustment * adj;
	Gtk::Frame value_frame;
};


#endif
