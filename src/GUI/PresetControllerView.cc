/*
 *  PresetControllerView.cc
 *
 *  Copyright (c) 2001-2012 Nick Dowell
 *
 *  This file is part of amsynth.
 *
 *  amsynth is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  amsynth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with amsynth.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PresetControllerView.h"

#include "../PresetController.h"
#include "../VoiceAllocationUnit.h"

#include <stdio.h>
#include <iostream>

extern Config config;

class PresetControllerViewImpl : public PresetControllerView, public UpdateListener
{
public:
	PresetControllerViewImpl(VoiceAllocationUnit *voiceAllocationUnit);

	virtual void setPresetController(PresetController *presetController);
	virtual void update();

private:

	static void on_combo_changed (GtkWidget *widget, PresetControllerViewImpl *);
	static void on_combo_popup_shown (GObject *gobject, GParamSpec *pspec, PresetControllerViewImpl *);
	static void on_save_clicked (GtkWidget *widget, PresetControllerViewImpl *);
	static void on_audition_pressed (GtkWidget *widget, PresetControllerViewImpl *);
	static void on_audition_released (GtkWidget *widget, PresetControllerViewImpl *);
	static void on_panic_clicked (GtkWidget *widget, PresetControllerViewImpl *);

	VoiceAllocationUnit *vau;
    PresetController *presetController;
	GtkWidget *combo;
	bool inhibit_combo_callback;
};

PresetControllerViewImpl::PresetControllerViewImpl(VoiceAllocationUnit *voiceAllocationUnit)
:	vau(voiceAllocationUnit)
,	presetController(NULL)
,	combo(NULL)
,	inhibit_combo_callback(false)
{
	combo = gtk_combo_box_new_text ();
	gtk_combo_box_set_wrap_width (GTK_COMBO_BOX (combo), 4);
	g_signal_connect (G_OBJECT (combo), "changed", G_CALLBACK (&PresetControllerViewImpl::on_combo_changed), this);
	g_signal_connect (G_OBJECT (combo), "notify::popup-shown", G_CALLBACK (&PresetControllerViewImpl::on_combo_popup_shown), this);
	add (* Glib::wrap (combo));
	
	GtkWidget *widget = NULL;
	
	widget = gtk_button_new_with_label ("Save");
	g_signal_connect (G_OBJECT (widget), "clicked", G_CALLBACK (&PresetControllerViewImpl::on_save_clicked), this);
	add (* Glib::wrap (widget));
	
	Gtk::Label *blank = manage (new Gtk::Label ("    "));
	add (*blank);
	
	widget = gtk_button_new_with_label ("Audition");
	g_signal_connect (G_OBJECT (widget), "pressed", G_CALLBACK (&PresetControllerViewImpl::on_audition_pressed), this);
	g_signal_connect (G_OBJECT (widget), "released", G_CALLBACK (&PresetControllerViewImpl::on_audition_released), this);
	add (* Glib::wrap (widget));
	
	widget = gtk_button_new_with_label ("Panic");
	g_signal_connect (G_OBJECT (widget), "clicked", G_CALLBACK (&PresetControllerViewImpl::on_panic_clicked), this);
	add (* Glib::wrap (widget));
}

void PresetControllerViewImpl::setPresetController(PresetController *presetController)
{
    this->presetController = presetController;
    update();
}

void PresetControllerViewImpl::on_combo_changed (GtkWidget *widget, PresetControllerViewImpl *that)
{
	if (that->inhibit_combo_callback)
		return;
	gint active = gtk_combo_box_get_active (GTK_COMBO_BOX (widget));
	that->presetController->selectPreset(active);
}

void PresetControllerViewImpl::on_combo_popup_shown (GObject *gobject, GParamSpec *pspec, PresetControllerViewImpl *that)
{
	const char *filename = config.current_bank_file.c_str();
	that->presetController->loadPresets(filename);
}

void PresetControllerViewImpl::on_save_clicked (GtkWidget *widget, PresetControllerViewImpl *that)
{
	const char *filename = config.current_bank_file.c_str();
	that->presetController->loadPresets(filename); // in case another instance has changed any of the other presets
	that->presetController->commitPreset();
	that->presetController->savePresets(filename);
	that->update();
}

void PresetControllerViewImpl::on_audition_pressed (GtkWidget *widget, PresetControllerViewImpl *that)
{
	that->vau->HandleMidiNoteOn(60, 1.0f);
}

void PresetControllerViewImpl::on_audition_released (GtkWidget *widget, PresetControllerViewImpl *that)
{
	that->vau->HandleMidiNoteOff(60, 0.0f);
}

void PresetControllerViewImpl::on_panic_clicked (GtkWidget *widget, PresetControllerViewImpl *that)
{
	that->vau->HandleMidiAllSoundOff();
}

void PresetControllerViewImpl::update()
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

PresetControllerView * PresetControllerView::create(VoiceAllocationUnit *voiceAllocationUnit)
{
	return new PresetControllerViewImpl(voiceAllocationUnit);
}
