/* amSynth
 * (c) 2001-2004 Nick Dowell
 */

#ifndef _PRESETCONTROLLERVIEW_H
#define _PRESETCONTROLLERVIEW_H

#include <gtkmm.h>
#include <string>
#include <list>

#include "../UpdateListener.h"

class PresetController;
class VoiceAllocationUnit;

class PresetControllerView : public UpdateListener, public Gtk::HBox {
public:
    PresetControllerView(VoiceAllocationUnit & vau );
    ~PresetControllerView();
    void setPresetController(PresetController & p_c);
    void update();
private:
	void ev_handler(std::string text);
    PresetController *presetController;
    Gtk::Button prev, next, commit;
    Gtk::Combo presets_combo;
    Gtk::Label preset_no_entry; // preset_name_entry;
	volatile bool inhibit_combo_callback, inhibit_combo_update;
	VoiceAllocationUnit *vau;
};

#endif
