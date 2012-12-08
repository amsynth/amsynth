/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "PresetControllerView.h"

#include "../PresetController.h"
#include "../VoiceAllocationUnit.h"

#include <stdio.h>
#include <iostream>

extern Config config;

PresetControllerView::PresetControllerView(VoiceAllocationUnit & vau )
:	vau(&vau)
,	presetController(NULL)
,	combo(NULL)
,	inhibit_combo_callback(false)
{
	combo = gtk_combo_box_new_text ();
	gtk_combo_box_set_wrap_width (GTK_COMBO_BOX (combo), 4);
	g_signal_connect (G_OBJECT (combo), "changed", G_CALLBACK (&PresetControllerView::on_combo_changed), this);
	g_signal_connect (G_OBJECT (combo), "notify::popup-shown", G_CALLBACK (&PresetControllerView::on_combo_popup_shown), this);
	add (* Glib::wrap (combo));
	
	GtkWidget *widget = NULL;
	
	widget = gtk_button_new_with_label ("Save");
	g_signal_connect (G_OBJECT (widget), "clicked", G_CALLBACK (&PresetControllerView::on_save_clicked), this);
	add (* Glib::wrap (widget));
	
	Gtk::Label *blank = manage (new Gtk::Label ("    "));
	add (*blank);
	
	widget = gtk_button_new_with_label ("Audition");
	g_signal_connect (G_OBJECT (widget), "pressed", G_CALLBACK (&PresetControllerView::on_audition_pressed), this);
	g_signal_connect (G_OBJECT (widget), "released", G_CALLBACK (&PresetControllerView::on_audition_released), this);
	add (* Glib::wrap (widget));
	
	widget = gtk_button_new_with_label ("Panic");
	g_signal_connect (G_OBJECT (widget), "clicked", G_CALLBACK (&PresetControllerView::on_panic_clicked), this);
	add (* Glib::wrap (widget));
}

PresetControllerView::~PresetControllerView()
{
}

void PresetControllerView::setPresetController(PresetController & p_c)
{
    presetController = &p_c;
    update();
}

void PresetControllerView::on_combo_changed (GtkWidget *widget, PresetControllerView *that)
{
	if (that->inhibit_combo_callback)
		return;
	gint active = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
	that->presetController->selectPreset(active);
}

void PresetControllerView::on_combo_popup_shown (GObject *gobject, GParamSpec *pspec, PresetControllerView *that)
{
	const char *filename = config.current_bank_file.c_str();
	that->presetController->loadPresets(filename);
}

void PresetControllerView::on_save_clicked (GtkWidget *widget, PresetControllerView *that)
{
	const char *filename = config.current_bank_file.c_str();
	that->presetController->loadPresets(filename); // in case another instance has changed any of the other presets
	that->presetController->commitPreset();
	that->presetController->savePresets(filename);
	that->update();
}

void PresetControllerView::on_audition_pressed (GtkWidget *widget, PresetControllerView *that)
{
	that->vau->HandleMidiNoteOn(60, 1.0f);
}

void PresetControllerView::on_audition_released (GtkWidget *widget, PresetControllerView *that)
{
	that->vau->HandleMidiNoteOff(60, 0.0f);
}

void PresetControllerView::on_panic_clicked (GtkWidget *widget, PresetControllerView *that)
{
	that->vau->HandleMidiAllSoundOff();
}

void PresetControllerView::update()
{
	inhibit_combo_callback = true;
	
	for (gint i = 0; i < PresetController::kNumPresets; i++) {
		gtk_combo_box_remove_text (GTK_COMBO_BOX (combo), 0);
	}
	char text [256] = "";
	for (gint i = 0; i < PresetController::kNumPresets; i++) {
		memset (text, 0, sizeof(text));
		sprintf (text, "%d: %s", i, presetController->getPreset(i).getName().c_str());
		gtk_combo_box_insert_text (GTK_COMBO_BOX (combo), i, text);
	}
	gtk_combo_box_set_active (GTK_COMBO_BOX (combo), presetController->getCurrPresetNumber());
	
	inhibit_combo_callback = false;
}
