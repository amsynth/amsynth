/* amSynth
 * (c) 2001-2004 Nick Dowell
 */

#ifndef _PRESETCONTROLLERVIEW_H
#define _PRESETCONTROLLERVIEW_H

#include <gtkmm.h>

#include "../UpdateListener.h"

class PresetController;
class VoiceAllocationUnit;

class PresetControllerView : public UpdateListener, public Gtk::HBox
{
public:
	
    PresetControllerView(VoiceAllocationUnit & vau );
    ~PresetControllerView();
	
    void setPresetController(PresetController & p_c);
	
    void update();
	
private:
	
	static void on_combo_changed (GtkWidget *widget, PresetControllerView *);
	static void on_combo_popup_shown (GObject *gobject, GParamSpec *pspec, PresetControllerView *);
	static void on_save_clicked (GtkWidget *widget, PresetControllerView *);
	static void on_audition_pressed (GtkWidget *widget, PresetControllerView *);
	static void on_audition_released (GtkWidget *widget, PresetControllerView *);
	static void on_panic_clicked (GtkWidget *widget, PresetControllerView *);

	VoiceAllocationUnit *vau;
    PresetController *presetController;
	GtkWidget *combo;
	bool inhibit_combo_callback;
};

#endif
