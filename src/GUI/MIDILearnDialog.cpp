/*
 *  MIDILearnDialog.cpp
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

#include <gtk/gtk.h>
#include "MIDILearnDialog.h"
#include "../MidiController.h"
#include "../PresetController.h"
#include "Request.h"
#include "controllers.h"

static gboolean on_output(GtkSpinButton *spin, gpointer data);

MIDILearnDialog::MIDILearnDialog(MidiController *midiController, PresetController *presetController, GtkWindow *parent)
:	_dialog(NULL)
,	_midiController(midiController)
,	_presetController(presetController)
{
	_dialog = gtk_dialog_new_with_buttons("MIDI Learn", parent, GTK_DIALOG_MODAL,
		GTK_STOCK_OK,     GTK_RESPONSE_ACCEPT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
		NULL);

	_ccSpinButton = gtk_spin_button_new_with_range(-1, 127, 1);
	gtk_spin_button_set_numeric(GTK_SPIN_BUTTON(_ccSpinButton), FALSE);
	g_signal_connect(G_OBJECT(_ccSpinButton), "output", (GCallback)on_output, NULL);

	_paramNameEntry = gtk_entry_new();
	gtk_entry_set_editable(GTK_ENTRY(_paramNameEntry), FALSE);

	GtkWidget *table = gtk_table_new(2, 2, FALSE);
	gtk_table_attach(GTK_TABLE(table), gtk_label_new("Synth Parameter:"), 0, 1, 0, 1, GTK_FILL, GTK_FILL, 5, 5);
	gtk_table_attach(GTK_TABLE(table), _paramNameEntry,                   1, 2, 0, 1, GTK_FILL, GTK_FILL, 5, 5);
	gtk_table_attach(GTK_TABLE(table), gtk_label_new("MIDI Controller"),  0, 1, 1, 2, GTK_FILL, GTK_FILL, 5, 5);
	gtk_table_attach(GTK_TABLE(table), _ccSpinButton,                     1, 2, 1, 2, GTK_FILL, GTK_FILL, 5, 5);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(_dialog)->vbox), table, TRUE, TRUE, 0);
}

MIDILearnDialog::~MIDILearnDialog()
{
}

void
MIDILearnDialog::run_modal(unsigned param_idx)
{
	Parameter &param = _presetController->getCurrentPreset().getParameter(param_idx);
	gtk_entry_set_text(GTK_ENTRY(_paramNameEntry), param.getName().c_str());
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(_ccSpinButton),
		_midiController->getControllerForParam(param.GetId()));
	
	_midiController->getLastControllerParam().addUpdateListener(*this);

	gtk_widget_show_all(_dialog);
	const gint response = gtk_dialog_run(GTK_DIALOG(_dialog));
	gtk_widget_hide(_dialog);
	
	if (response == GTK_RESPONSE_ACCEPT) {
		int midi_cc = gtk_spin_button_get_value(GTK_SPIN_BUTTON(_ccSpinButton));
		if (-1 <= midi_cc && midi_cc < 127)
			_midiController->setController(midi_cc, param);
	}

	_midiController->getLastControllerParam().removeUpdateListener(*this);
}

void
MIDILearnDialog::update()
{
	CALL_ON_GUI_THREAD(*this, &MIDILearnDialog::last_active_controller_changed);
}

void
MIDILearnDialog::last_active_controller_changed()
{
	int value = (int)_midiController->getLastControllerParam().getValue();
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(_ccSpinButton), value);
}

static gboolean on_output(GtkSpinButton *spin, gpointer)
{
   const int value = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin));
   const gchar *text = (0 <= value && value < 128) ? c_controller_names[value] : "None";
   gtk_entry_set_text(GTK_ENTRY(spin), text);
   return TRUE;
}

