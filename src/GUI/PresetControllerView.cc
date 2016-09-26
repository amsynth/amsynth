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
#include "../main.h"
#include "../midi.h"

#include <fstream>
#include <glib/gi18n.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

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
	PresetControllerViewImpl();

	virtual void setPresetController(PresetController *presetController);
	virtual void update();
	virtual int getAuditionNote();

private:

	static void on_combo_changed (GtkWidget *widget, PresetControllerViewImpl *);
	static void on_combo_popup_shown (GObject *gobject, GParamSpec *pspec, PresetControllerViewImpl *);
	static void on_save_clicked (GtkWidget *widget, PresetControllerViewImpl *);
	static void on_audition_pressed (GtkWidget *widget, PresetControllerViewImpl *);
	static void on_audition_released (GtkWidget *widget, PresetControllerViewImpl *);
	static void on_panic_clicked (GtkWidget *widget, PresetControllerViewImpl *);
    
    static gboolean on_audition_key_press_event (GtkWidget *widget, GdkEventKey *event, PresetControllerViewImpl *);
    static gboolean on_audition_key_release_event (GtkWidget *widget, GdkEventKey *event, PresetControllerViewImpl *);

    PresetController *presetController;
	GtkWidget *bank_combo;
	GtkWidget *combo;
	GtkWidget *save_button;
	GtkWidget *audition_spin;
	int audition_note;
	bool inhibit_combo_callback;
    bool audition_button_pressed;
};

static GtkWidget * button_with_image(const gchar *stock_id, const gchar *label)
{
	if (!gtk_icon_factory_lookup_default (stock_id)) {
		return gtk_button_new_with_label (label);
	}
	GtkWidget *button = gtk_button_new ();
	gtk_button_set_image (GTK_BUTTON (button), gtk_image_new_from_stock (stock_id, GTK_ICON_SIZE_SMALL_TOOLBAR));
	atk_object_set_name (gtk_widget_get_accessible (button), label);
	return button;
}

PresetControllerViewImpl::PresetControllerViewImpl()
:	presetController(NULL)
,	bank_combo(NULL)
,	combo(NULL)
,	audition_spin(NULL)
,	audition_note(0)
,	inhibit_combo_callback(false)
{
	bank_combo = gtk_combo_box_new_text ();
	g_signal_connect (G_OBJECT (bank_combo), "changed", G_CALLBACK (&PresetControllerViewImpl::on_combo_changed), this);
	pack_start (* Glib::wrap (bank_combo), false, false);

	combo = gtk_combo_box_new_text ();
	gtk_combo_box_set_wrap_width (GTK_COMBO_BOX (combo), 4);
	g_signal_connect (G_OBJECT (combo), "changed", G_CALLBACK (&PresetControllerViewImpl::on_combo_changed), this);
	g_signal_connect (G_OBJECT (combo), "notify::popup-shown", G_CALLBACK (&PresetControllerViewImpl::on_combo_popup_shown), this);
	pack_start (* Glib::wrap (combo), true, true);

	save_button = button_with_image (GTK_STOCK_SAVE, _("Save"));
	g_signal_connect (G_OBJECT (save_button), "clicked", G_CALLBACK (&PresetControllerViewImpl::on_save_clicked), this);
	pack_start (* Glib::wrap (save_button), false, false);
	
	Gtk::Label *blank = manage (new Gtk::Label ("  "));
	pack_start (*blank, false, false);
	
	GtkWidget *widget = NULL;

	widget = button_with_image (GTK_STOCK_MEDIA_PLAY, _("Audition"));
	g_signal_connect (G_OBJECT (widget), "pressed", G_CALLBACK (&PresetControllerViewImpl::on_audition_pressed), this);
	g_signal_connect (G_OBJECT (widget), "released", G_CALLBACK (&PresetControllerViewImpl::on_audition_released), this);
    g_signal_connect (G_OBJECT (widget), "key-press-event", G_CALLBACK (&PresetControllerViewImpl::on_audition_key_press_event), this);
    g_signal_connect (G_OBJECT (widget), "key-release-event", G_CALLBACK (&PresetControllerViewImpl::on_audition_key_release_event), this);
	pack_start (* Glib::wrap (widget), false, false);
	
	GtkAdjustment *audition_adj = (GtkAdjustment *) gtk_adjustment_new(60.0, 0.0, 127.0, 1.0, 5.0, 0.0);
	audition_spin = gtk_spin_button_new(audition_adj, 1.0, 0);
	pack_start (* Glib::wrap (audition_spin), false, false);

	widget = button_with_image (GTK_STOCK_MEDIA_STOP, _("Panic"));
	g_signal_connect (G_OBJECT (widget), "clicked", G_CALLBACK (&PresetControllerViewImpl::on_panic_clicked), this);
	pack_start (* Glib::wrap (widget), false, false);
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
	that->audition_note = that->getAuditionNote();
	amsynth_midi_input(MIDI_STATUS_NOTE_ON, that->audition_note, 127);
}

void PresetControllerViewImpl::on_audition_released (GtkWidget *widget, PresetControllerViewImpl *that)
{
	amsynth_midi_input(MIDI_STATUS_NOTE_OFF, that->audition_note, 0);
	that->audition_note = 0;
}

gboolean PresetControllerViewImpl::on_audition_key_press_event(GtkWidget *widget, GdkEventKey *event, PresetControllerViewImpl *that)
{
    if (event->keyval == GDK_space || event->keyval == GDK_Return) {
        if (!that->audition_button_pressed) {
            that->audition_button_pressed = TRUE;
            gtk_button_pressed(GTK_BUTTON(widget));
        }
        return TRUE;
    }
    return FALSE;
}

gboolean PresetControllerViewImpl::on_audition_key_release_event(GtkWidget *widget, GdkEventKey *event, PresetControllerViewImpl *that)
{
    if (event->keyval == GDK_space || event->keyval == GDK_Return) {
        if (that->audition_button_pressed) {
            that->audition_button_pressed = FALSE;
            gtk_button_released(GTK_BUTTON(widget));
        }
        return TRUE;
    }
    return FALSE;
}

void PresetControllerViewImpl::on_panic_clicked (GtkWidget *widget, PresetControllerViewImpl *that)
{
	amsynth_midi_input(MIDI_STATUS_CONTROLLER, MIDI_CC_ALL_SOUND_OFF, 0);
}

void PresetControllerViewImpl::update()
{
	inhibit_combo_callback = true;

	const std::vector<BankInfo> banks = PresetController::getPresetBanks();

	char text [48] = "";

	// bank combo

	gtk_list_store_clear (GTK_LIST_STORE (gtk_combo_box_get_model (GTK_COMBO_BOX (bank_combo))));
	for (size_t i=0; i<banks.size(); i++) {
		snprintf_truncate (text, sizeof(text), "[%s] %s", banks[i].read_only ? _("F") : _("U"), banks[i].name.c_str());
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

	// allow the preset combo to be narrower than the longest preset name, prevents the window getting wider than the editor
	gtk_widget_set_size_request (combo, 100, -1);

	inhibit_combo_callback = false;
}

int PresetControllerViewImpl::getAuditionNote()
{
	audition_note = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(audition_spin));
	return audition_note;
}

PresetControllerView * PresetControllerView::create()
{
	return new PresetControllerViewImpl();
}
