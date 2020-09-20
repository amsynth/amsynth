/*
 *  ConfigDialog.cpp
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

#include "ConfigDialog.h"

#include "../Configuration.h"

#include <cstdarg>
#include <glib/gi18n.h>
#include <stdlib.h>
#include <strings.h>


static GtkWidget *
make_combo(const char *selected, const char *text1, ...)
{
	GtkWidget *widget = gtk_combo_box_text_new();

	va_list args;
	va_start(args, text1);
	gint i = 0, active = 0;
	const char *text = text1;
	while (text) {
		gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(widget), text);
		if (strcasecmp(text, selected) == 0)
			active = i;
		text = va_arg(args, const char *);
		i++;
	}
	va_end(args);

	gtk_combo_box_set_active(GTK_COMBO_BOX(widget), active);

	return widget;
}

void config_dialog_run(GtkWindow *parent)
{
	Configuration & config = Configuration::get();

	GtkWidget *dialog = gtk_dialog_new_with_buttons(
			_("amsynth configuration"), parent, GTK_DIALOG_MODAL,
			GTK_STOCK_OK,     GTK_RESPONSE_ACCEPT,
			GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
			NULL);

	GtkWidget *widget = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
	GtkBox *vbox = GTK_BOX(widget);

	gtk_box_pack_start(vbox, gtk_label_new(_("Preferred MIDI Driver")), TRUE, 0, 0);
	GtkWidget *midi_combo = make_combo(config.midi_driver.c_str(), "auto", "oss", "alsa", NULL);
	gtk_box_pack_start(vbox, midi_combo, TRUE, 0, 0);

	gtk_box_pack_start(vbox, gtk_label_new(_("OSS MIDI Device")), TRUE, 0, 0);
	GtkWidget *oss_midi_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(oss_midi_entry), config.oss_midi_device.c_str());
	gtk_box_pack_start(vbox, oss_midi_entry, TRUE, 0, 0);

	gtk_box_pack_start(vbox, gtk_label_new(_("Preferred Audio Driver")), TRUE, 0, 0);
	GtkWidget *audio_combo = make_combo(config.audio_driver.c_str(), "auto", "jack", "alsa-mmap", "alsa", "oss", NULL);
	gtk_box_pack_start(vbox, audio_combo, TRUE, 0, 0);

	gtk_box_pack_start(vbox, gtk_label_new(_("OSS Audio Device")), TRUE, 0, 0);
	GtkWidget *oss_audio_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(oss_audio_entry), config.oss_audio_device.c_str());
	gtk_box_pack_start(vbox, oss_audio_entry, TRUE, 0, 0);

	gtk_box_pack_start(vbox, gtk_label_new(_("ALSA Audio Device")), TRUE, 0, 0);
	GtkWidget *alsa_audio_entry = gtk_entry_new();
	gtk_entry_set_text(GTK_ENTRY(alsa_audio_entry), config.alsa_audio_device.c_str());
	gtk_box_pack_start(vbox, alsa_audio_entry, TRUE, 0, 0);

	gtk_box_pack_start(vbox, gtk_label_new(_("Sample Rate")), TRUE, 0, 0);
	char sample_rate[10] = "";
	sprintf(sample_rate, "%d", config.sample_rate);
	GtkWidget *sample_rate_combo = make_combo(sample_rate, "22050", "44100", "48000", "88200", "96000", "176400", "192000", NULL);

	gtk_box_pack_start(vbox, sample_rate_combo, TRUE, 0, 0);

	gtk_box_pack_start(vbox, gtk_label_new(_("JACK")), TRUE, 0, 0);
	GtkWidget *jack_autoconnect = gtk_check_button_new_with_label(_("Automatically connect to system outputs"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(jack_autoconnect), config.jack_autoconnect);
	gtk_box_pack_start(vbox, jack_autoconnect, TRUE, 0, 0);

	gtk_box_pack_start(vbox, gtk_label_new(_("")), TRUE, 0, 0);
	gtk_box_pack_start(vbox, gtk_label_new(_("Changes take effect after restarting amsynth")), TRUE, 0, 0);

	gtk_widget_show_all(dialog);

	const gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	if (response == GTK_RESPONSE_ACCEPT) {
		config.midi_driver = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(midi_combo));
		config.oss_midi_device = gtk_entry_get_text(GTK_ENTRY(oss_midi_entry));
		config.audio_driver = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(audio_combo));
		config.oss_audio_device = gtk_entry_get_text(GTK_ENTRY(oss_audio_entry));
		config.alsa_audio_device = gtk_entry_get_text(GTK_ENTRY(alsa_audio_entry));
		config.sample_rate = (int) strtol(gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(sample_rate_combo)), nullptr, 0);
		config.jack_autoconnect = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(jack_autoconnect));
		config.save();
	}

	gtk_widget_destroy(dialog);
}
