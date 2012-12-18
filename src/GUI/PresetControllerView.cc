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


//////////

struct BankInfo {
	std::string name;
	std::string file_path;
	bool read_only;
};

static std::vector<BankInfo> s_banks;

static void scan_preset_bank(const std::string dir_path, const std::string file_name, bool read_only)
{
	std::string file_path = dir_path + std::string("/") + std::string(file_name);

	std::string bank_name = std::string(file_name);
	if (bank_name == std::string(".amSynth.presets")) {
		bank_name = "User bank";
	} else {
		std::string::size_type pos = bank_name.find_first_of(".");
		if (pos != std::string::npos)
			bank_name.erase(pos, string::npos);
	}

	PresetController preset_controller;
	if (preset_controller.loadPresets(file_path.c_str()) != 0)
		return;

	BankInfo bank_info;
	bank_info.name = bank_name;
	bank_info.file_path = file_path;
	bank_info.read_only = read_only;
	s_banks.push_back(bank_info);
}

static void scan_preset_banks(const std::string dir_path, bool read_only)
{
	GDir *gdir = g_dir_open(dir_path.c_str(), 0, NULL);
	if (!gdir)
		return;

	const gchar *file_name = NULL;
	while ((file_name = g_dir_read_name(gdir))) {
		scan_preset_bank(dir_path, file_name, read_only);
	}

	g_dir_close(gdir);
}

static void scan_preset_banks()
{
	s_banks.clear();
	scan_preset_bank(std::string(getenv("HOME")), ".amSynth.presets", false);
	scan_preset_banks(std::string(getenv("HOME")) + std::string("/.amsynth/banks"), false);
	scan_preset_banks(std::string(PKGDATADIR "/banks"), true);
}

//////////


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
	bool inhibit_combo_callback;
};

PresetControllerViewImpl::PresetControllerViewImpl(VoiceAllocationUnit *voiceAllocationUnit)
:	vau(voiceAllocationUnit)
,	presetController(NULL)
,	bank_combo(NULL)
,	combo(NULL)
,	inhibit_combo_callback(false)
{
	bank_combo = gtk_combo_box_new_text ();
	scan_preset_banks ();
	for (size_t i=0; i<s_banks.size(); i++) {
		char text [64] = "";
		snprintf (text, sizeof(text), "[%s] %s", s_banks[i].read_only ? "factory" : "user", s_banks[i].name.c_str());
		gtk_combo_box_insert_text (GTK_COMBO_BOX (bank_combo), i, text);
	}
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
	
	Gtk::Label *blank = manage (new Gtk::Label ("    "));
	add (*blank);
	
	GtkWidget *widget = NULL;

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

	if (widget == that->bank_combo) {
		gint bank = gtk_combo_box_get_active (GTK_COMBO_BOX (that->bank_combo));
		that->presetController->loadPresets(s_banks[bank].file_path.c_str());
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

	const std::string current_file_path = presetController->getFilePath();
	for (size_t i=0; i<s_banks.size(); i++) {
		if (current_file_path == s_banks[i].file_path) {
			gtk_combo_box_set_active (GTK_COMBO_BOX (bank_combo), i);
			gtk_widget_set_sensitive (save_button, !s_banks[i].read_only);
		}
	}
	
	inhibit_combo_callback = false;
}

PresetControllerView * PresetControllerView::create(VoiceAllocationUnit *voiceAllocationUnit)
{
	return new PresetControllerViewImpl(voiceAllocationUnit);
}
