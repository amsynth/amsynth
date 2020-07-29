/*
 *  PresetControllerView.cpp
 *
 *  Copyright (c) 2001-2019 Nick Dowell
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
#include "../main.h"
#include "../midi.h"

#include <fstream>
#include <gdk/gdkkeysyms.h>
#include <glib/gi18n.h>
#include <iostream>
#include <stdio.h>
#include <string>
#include <vector>

static void snprintf_truncate(char *str, size_t size, const char *format, ...)
{
	va_list va_args;
	va_start(va_args, format);
	size_t count = vsnprintf(str, size, format, va_args);
	va_end(va_args);
	if (count >= size - 1)
		strcpy(str + size - 4, "...");
}

class PresetControllerViewImpl : public PresetControllerView, public UpdateListener
{
public:
	PresetControllerViewImpl(PresetController *presetController);

	void update() override;
	unsigned char getAuditionNote() override;
	GtkWidget * getWidget() override { return widget; }

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
	GtkWidget *widget;
	GtkWidget *bank_combo;
	GtkWidget *combo;
	GtkWidget *save_button;
	GtkWidget *audition_spin;
	unsigned char audition_note;
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

static gboolean on_output(GtkSpinButton *spin, gpointer user_data)
{
	static const char *names[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
	gchar text[12];
	gint value = gtk_spin_button_get_value_as_int (spin);
	sprintf (text, "%s%d", names[value % 12], value / 12 - 1);
	gtk_entry_set_text (GTK_ENTRY (spin), text);
	return TRUE;
}

PresetControllerViewImpl::PresetControllerViewImpl(PresetController *presetController)
:	presetController(presetController)
,	widget(nullptr)
,	bank_combo(nullptr)
,	combo(nullptr)
,	audition_spin(nullptr)
,	audition_note(0)
,	inhibit_combo_callback(false)
{
	this->widget = gtk_hbox_new (FALSE, 0);
	GtkBox *hbox = GTK_BOX (this->widget);

	bank_combo = gtk_combo_box_text_new ();
	g_signal_connect (G_OBJECT (bank_combo), "changed", G_CALLBACK (&PresetControllerViewImpl::on_combo_changed), this);
	gtk_box_pack_start (hbox, bank_combo, FALSE, FALSE, 0);

	combo = gtk_combo_box_text_new ();
	gtk_combo_box_set_wrap_width (GTK_COMBO_BOX (combo), 4);
	g_signal_connect (G_OBJECT (combo), "changed", G_CALLBACK (&PresetControllerViewImpl::on_combo_changed), this);
	g_signal_connect (G_OBJECT (combo), "notify::popup-shown", G_CALLBACK (&PresetControllerViewImpl::on_combo_popup_shown), this);
	gtk_box_pack_start (hbox, combo, TRUE, TRUE, 0);

	save_button = button_with_image (GTK_STOCK_SAVE, _("Save"));
	g_signal_connect (G_OBJECT (save_button), "clicked", G_CALLBACK (&PresetControllerViewImpl::on_save_clicked), this);
	gtk_box_pack_start (hbox, save_button, FALSE, FALSE, 0);

	gtk_box_pack_start (hbox, gtk_label_new ("  "), FALSE, FALSE, 0);

	GtkWidget *widget = nullptr;

	widget = button_with_image (GTK_STOCK_MEDIA_PLAY, _("Audition"));
	g_signal_connect (G_OBJECT (widget), "pressed", G_CALLBACK (&PresetControllerViewImpl::on_audition_pressed), this);
	g_signal_connect (G_OBJECT (widget), "released", G_CALLBACK (&PresetControllerViewImpl::on_audition_released), this);
    g_signal_connect (G_OBJECT (widget), "key-press-event", G_CALLBACK (&PresetControllerViewImpl::on_audition_key_press_event), this);
    g_signal_connect (G_OBJECT (widget), "key-release-event", G_CALLBACK (&PresetControllerViewImpl::on_audition_key_release_event), this);
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 0);
	
	GtkAdjustment *audition_adj = (GtkAdjustment *) gtk_adjustment_new(60.0, 0.0, 127.0, 1.0, 5.0, 0.0);
	audition_spin = gtk_spin_button_new(audition_adj, 1.0, 0);
	gtk_editable_set_editable (GTK_EDITABLE (audition_spin), false);
	gtk_widget_set_can_focus (audition_spin, FALSE);
	gtk_spin_button_set_numeric (GTK_SPIN_BUTTON (audition_spin), false);
	g_signal_connect (G_OBJECT (audition_spin), "output", G_CALLBACK (&on_output), NULL);
	gtk_box_pack_start (hbox, audition_spin, FALSE, FALSE, 0);

	widget = button_with_image (GTK_STOCK_MEDIA_STOP, _("Panic"));
	g_signal_connect (G_OBJECT (widget), "clicked", G_CALLBACK (&PresetControllerViewImpl::on_panic_clicked), this);
	gtk_box_pack_start (hbox, widget, FALSE, FALSE, 0);

	update();
}

void PresetControllerViewImpl::on_combo_changed (GtkWidget *widget, PresetControllerViewImpl *that)
{
	if (that->inhibit_combo_callback)
		return;

	if (widget == that->bank_combo) {
		gint bank = gtk_combo_box_get_active (GTK_COMBO_BOX (that->bank_combo));
		const std::vector<BankInfo> &banks = PresetController::getPresetBanks();
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
            g_signal_emit_by_name (widget, "pressed", 0);
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
            g_signal_emit_by_name (widget, "released", 0);
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

	const std::vector<BankInfo> &banks = PresetController::getPresetBanks();

	char text [48] = "";

	// bank combo

	gtk_list_store_clear (GTK_LIST_STORE (gtk_combo_box_get_model (GTK_COMBO_BOX (bank_combo))));
	for (size_t i=0; i<banks.size(); i++) {
		snprintf_truncate (text, sizeof(text), "[%s] %s", banks[i].read_only ? _("F") : _("U"), banks[i].name.c_str());
		gtk_combo_box_text_insert_text (GTK_COMBO_BOX_TEXT (bank_combo), (gint) i, text);
	}

	const std::string current_file_path = presetController->getFilePath();
	for (size_t i=0; i<banks.size(); i++) {
		if (current_file_path == banks[i].file_path) {
			gtk_combo_box_set_active (GTK_COMBO_BOX (bank_combo), (gint) i);
			gtk_widget_set_sensitive (save_button, !banks[i].read_only);
		}
	}

	// preset combo

	gtk_list_store_clear (GTK_LIST_STORE (gtk_combo_box_get_model (GTK_COMBO_BOX (combo))));
	
	for (gint i = 0; i < PresetController::kNumPresets; i++) {
		snprintf_truncate (text, sizeof(text), "%d: %s", i, presetController->getPreset(i).getName().c_str());
		gtk_combo_box_text_insert_text (GTK_COMBO_BOX_TEXT (combo), i, text);
	}
	gtk_combo_box_set_active (GTK_COMBO_BOX (combo), presetController->getCurrPresetNumber());

	// allow the preset combo to be narrower than the longest preset name, prevents the window getting wider than the editor
	gtk_widget_set_size_request (combo, 100, -1);

	inhibit_combo_callback = false;
}

unsigned char PresetControllerViewImpl::getAuditionNote()
{
	audition_note = (unsigned char) gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(audition_spin));
	return audition_note;
}

PresetControllerView * PresetControllerView::instantiate(PresetController *presetController)
{
    return new PresetControllerViewImpl(presetController);
}
