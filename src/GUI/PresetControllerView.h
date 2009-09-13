/* amSynth
 * (c) 2001-2004 Nick Dowell
 */

#ifndef _PRESETCONTROLLERVIEW_H
#define _PRESETCONTROLLERVIEW_H

#include <gtkmm.h>
#include <string>
#include <list>
#include "ParameterView.h"

class PresetController;
class UpdateListener;
class VoiceAllocationUnit;

class PresetControllerView : public UpdateListener, public Gtk::HBox {
public:
    PresetControllerView(int pipe_d, VoiceAllocationUnit & vau );
    ~PresetControllerView();
    void setPresetController(PresetController & p_c);
    void update();
private:
	void ev_handler(string text);
    PresetController *presetController;
    Gtk::Button prev, next, commit;
    Gtk::Combo presets_combo;
    Gtk::Label preset_no_entry; // preset_name_entry;
	volatile bool inhibit_combo_callback, inhibit_combo_update;
	int piped;
	VoiceAllocationUnit *vau;
};

#endif
