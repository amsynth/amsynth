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

#include "MIDILearnDialog.h"

#include "../MidiController.h"
#include "../PresetController.h"
#include "controllers.h"

#include <cassert>
#include <glib/gi18n.h>


MIDILearnDialog::MIDILearnDialog(MidiController *midiController, PresetController *presetController, GtkWindow *parent)
:	_dialog(NULL)
,	_midiController(midiController)
,	_presetController(presetController)
{
	_dialog = gtk_dialog_new_with_buttons(_("MIDI Learn"), parent, GTK_DIALOG_MODAL,
		GTK_STOCK_OK,     GTK_RESPONSE_ACCEPT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
		NULL);

	_paramNameEntry = gtk_entry_new();
	gtk_entry_set_editable(GTK_ENTRY(_paramNameEntry), FALSE);

	_combo = gtk_combo_box_new_text();
	gtk_combo_box_set_wrap_width (GTK_COMBO_BOX (_combo), 4);
	gtk_combo_box_insert_text (GTK_COMBO_BOX (_combo), 0, _("None"));
	for (gint i = 0; i < 128; i++)
		gtk_combo_box_insert_text (GTK_COMBO_BOX (_combo), i + 1, c_controller_names[i]);

	GtkWidget *table = gtk_table_new(2, 2, FALSE);
	gtk_table_attach(GTK_TABLE(table), gtk_label_new(_("Synth Parameter:")), 0, 1, 0, 1, GTK_FILL, GTK_FILL, 5, 5);
	gtk_table_attach(GTK_TABLE(table), _paramNameEntry,                   1, 2, 0, 1, GTK_FILL, GTK_FILL, 5, 5);
	gtk_table_attach(GTK_TABLE(table), gtk_label_new(_("MIDI Controller")),  0, 1, 1, 2, GTK_FILL, GTK_FILL, 5, 5);
	gtk_table_attach(GTK_TABLE(table), _combo,                            1, 2, 1, 2, GTK_FILL, GTK_FILL, 5, 5);

	gtk_box_pack_start(GTK_BOX(GTK_DIALOG(_dialog)->vbox), table, TRUE, TRUE, 0);

	_midiController->getLastControllerParam().addUpdateListener(*this);
}

MIDILearnDialog::~MIDILearnDialog()
{
	_midiController->getLastControllerParam().removeUpdateListener(*this);
}

void
MIDILearnDialog::run_modal(Param param_idx)
{
	int cc = _midiController->getControllerForParameter(param_idx);
	gtk_entry_set_text(GTK_ENTRY(_paramNameEntry), parameter_name_from_index (param_idx));
	gtk_combo_box_set_active (GTK_COMBO_BOX (_combo), cc + 1);

	gtk_widget_show_all(_dialog);
	const gint response = gtk_dialog_run(GTK_DIALOG(_dialog));
	gtk_widget_hide(_dialog);
	
	if (response == GTK_RESPONSE_ACCEPT) {
		cc = gtk_combo_box_get_active(GTK_COMBO_BOX (_combo)) - 1;
		_midiController->setControllerForParameter(param_idx, cc);
	}
}

void
MIDILearnDialog::update()
{
	gdk_threads_enter();
	last_active_controller_changed();
	gdk_threads_leave();
}

void
MIDILearnDialog::last_active_controller_changed()
{
	int cc = (int)_midiController->getLastControllerParam().getValue();
	gtk_combo_box_set_active (GTK_COMBO_BOX (_combo), cc + 1);
}

static gboolean on_output(GtkSpinButton *spin, gpointer)
{
	int cc = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin)) - 1;
	const gchar *text = cc < 0 ? _("None") : c_controller_names[cc];
	gtk_entry_set_text(GTK_ENTRY(spin), text);
	return TRUE;
}
