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

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

extern Config config;

static void snprintf_truncate(char *str, size_t size, const char *format, ...)
{
	va_list va_args;
	va_start(va_args, format);
	int count = vsnprintf(str, size, format, va_args);
	va_end(va_args);
	if (count >= size - 1)
		strcpy(str + size - 4, "...");
}

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
	GtkWidget *bank_combo;
	GtkWidget *combo;
	GtkWidget *save_button;
	GtkWidget *audition_spin;
	int audition_note;
	bool inhibit_combo_callback;
};

PresetControllerViewImpl::PresetControllerViewImpl(VoiceAllocationUnit *voiceAllocationUnit)
:	vau(voiceAllocationUnit)
,	presetController(NULL)
,	bank_combo(NULL)
,	combo(NULL)
,	audition_spin(NULL)
,	audition_note(0)
,	inhibit_combo_callback(false)
{
	bank_combo = gtk_combo_box_new_text ();
	g_signal_connect (G_OBJECT (bank_combo), "changed", G_CALLBACK (&PresetControllerViewImpl::on_combo_changed), this);
	add (* Glib::wrap (bank_combo));

	combo = gtk_combo_box_new_text ();
	gtk_combo_box_set_wrap_width (GTK_COMBO_BOX (combo), 4);
	g_signal_connect (G_OBJECT (combo), "changed", G_CALLBACK (&PresetControllerViewImpl::on_combo_changed), this);
	g_signal_connect (G_OBJECT (combo), "notify::popup-shown", G_CALLBACK (&PresetControllerViewImpl::on_combo_popup_shown), this);
	add (* Glib::wrap (combo));
	
	save_button = gtk_button_new_with_label ("Save");
	g_signal_connect (G_OBJECT (save_button), "clicked", G_CALLBACK (&PresetControllerViewImpl::on_save_clicked), this);
	add (* Glib::wrap (save_button));
	
	Gtk::Label *blank = manage (new Gtk::Label ("  "));
	add (*blank);
	
	GtkWidget *widget = NULL;

	widget = gtk_button_new_with_label ("Audition");
	g_signal_connect (G_OBJECT (widget), "pressed", G_CALLBACK (&PresetControllerViewImpl::on_audition_pressed), this);
	g_signal_connect (G_OBJECT (widget), "released", G_CALLBACK (&PresetControllerViewImpl::on_audition_released), this);
	add (* Glib::wrap (widget));
	
	GtkAdjustment *audition_adj = (GtkAdjustment *) gtk_adjustment_new(60.0, 0.0, 127.0, 1.0, 5.0, 0.0);
	audition_spin = gtk_spin_button_new(audition_adj, 1.0, 0);
	add (* Glib::wrap (audition_spin));

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

	if (widget == that->bank_combo) {
		gint bank = gtk_combo_box_get_active (GTK_COMBO_BOX (that->bank_combo));
		const std::vector<BankInfo> banks = PresetController::getPresetBanks();
		that->presetController->loadPresets(banks[bank].file_path.c_str());
	}

	gint preset = gtk_combo_box_get_active (GTK_COMBO_BOX (that->combo));
	that->presetController->selectPreset (PresetController::kNumPresets - preset - 1);
	that->presetController->selectPreset (preset);
}

void PresetControllerViewImpl::on_combo_popup_shown (GObject *gobject, GParamSpec *pspec, PresetControllerViewImpl *that)
{
	that->presetController->loadPresets();
}

void PresetControllerViewImpl::on_save_clicked (GtkWidget *widget, PresetControllerViewImpl *that)
{
	that->presetController->loadPresets(); // in case another instance has changed any of the other presets
	that->presetController->commitPreset();
	that->presetController->savePresets();
	that->update();
}

void PresetControllerViewImpl::on_audition_pressed (GtkWidget *widget, PresetControllerViewImpl *that)
{
	that->audition_note = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(that->audition_spin));
	that->vau->HandleMidiNoteOn(that->audition_note, 1.0f);
}

void PresetControllerViewImpl::on_audition_released (GtkWidget *widget, PresetControllerViewImpl *that)
{
	that->vau->HandleMidiNoteOff(that->audition_note, 0.0f);
}

void PresetControllerViewImpl::on_panic_clicked (GtkWidget *widget, PresetControllerViewImpl *that)
{
	that->vau->HandleMidiAllSoundOff();
}

void PresetControllerViewImpl::update()
{
	inhibit_combo_callback = true;

	const std::vector<BankInfo> banks = PresetController::getPresetBanks();

	char text [22] = "";

	// bank combo

	gtk_list_store_clear (GTK_LIST_STORE (gtk_combo_box_get_model (GTK_COMBO_BOX (bank_combo))));
	for (size_t i=0; i<banks.size(); i++) {
		snprintf_truncate (text, sizeof(text), "[%s] %s", banks[i].read_only ? "F" : "U", banks[i].name.c_str());
		gtk_combo_box_insert_text (GTK_COMBO_BOX (bank_combo), i, text);
	}

	const std::string current_file_path = presetController->getFilePath();
	for (size_t i=0; i<banks.size(); i++) {
		if (current_file_path == banks[i].file_path) {
			gtk_combo_box_set_active (GTK_COMBO_BOX (bank_combo), i);
			gtk_widget_set_sensitive (save_button, !banks[i].read_only);
		}
	}

	// preset combo

	gtk_list_store_clear (GTK_LIST_STORE (gtk_combo_box_get_model (GTK_COMBO_BOX (combo))));
	
	for (gint i = 0; i < PresetController::kNumPresets; i++) {
		snprintf_truncate (text, sizeof(text), "%d: %s", i, presetController->getPreset(i).getName().c_str());
		gtk_combo_box_insert_text (GTK_COMBO_BOX (combo), i, text);
	}
	gtk_combo_box_set_active (GTK_COMBO_BOX (combo), presetController->getCurrPresetNumber());
	
	inhibit_combo_callback = false;
}

PresetControllerView * PresetControllerView::create(VoiceAllocationUnit *voiceAllocationUnit)
{
	return new PresetControllerViewImpl(voiceAllocationUnit);
}
