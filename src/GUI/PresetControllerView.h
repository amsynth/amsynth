/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#ifndef _PRESETCONTROLLERVIEW_H
#define _PRESETCONTROLLERVIEW_H

#include <gtk--/box.h>
#include <gtk--/button.h>
#include <gtk--/combo.h>
#include <gtk--/label.h>
#include <string>
#include <list>
#include "ParameterView.h"
#include "../PresetController.h"
#include "../UpdateListener.h"

class PresetControllerView : public UpdateListener, public Gtk::HBox {
public:
    PresetControllerView( int pipe_d );
    ~PresetControllerView();
    void setPresetController(PresetController & p_c);
    void update();
	void _update_();
private:
	void ev_handler(string text);
    PresetController *presetController;
    Gtk::Button prev, next, commit;
    Gtk::Combo presets_combo;
    Gtk::Label preset_no_entry; // preset_name_entry;
	volatile bool inhibit_combo_callback, inhibit_combo_update;
	int piped;
	Request request;
};

#endif
