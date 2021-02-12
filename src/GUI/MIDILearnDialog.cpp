/*
 *  MIDILearnDialog.cpp
 *
 *  Copyright (c) 2001-2020 Nick Dowell
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

#include <cassert>
#include <glib/gi18n.h>
#include <seq24/controllers.h>


MIDILearnDialog::MIDILearnDialog(MidiController *midiController, PresetController *presetController, GtkWindow *parent)
:	_dialog(nullptr)
,	_midiController(midiController)
,	_presetController(presetController)
{
	_dialog = gtk_dialog_new_with_buttons(_("MIDI Controller Assignment"), parent, GTK_DIALOG_MODAL,
		GTK_STOCK_OK,     GTK_RESPONSE_ACCEPT,
		GTK_STOCK_CANCEL, GTK_RESPONSE_REJECT,
		NULL);

	_paramNameEntry = gtk_entry_new();
	gtk_editable_set_editable(GTK_EDITABLE(_paramNameEntry), FALSE);

	_combo = gtk_combo_box_text_new();
	gtk_combo_box_set_wrap_width (GTK_COMBO_BOX (_combo), 4);
	gtk_combo_box_text_insert_text (GTK_COMBO_BOX_TEXT (_combo), 0, _("None"));
	for (gint i = 0; i < 128; i++)
		gtk_combo_box_text_insert_text (GTK_COMBO_BOX_TEXT (_combo), i + 1, c_controller_names[i]);

	_checkButton = gtk_check_button_new_with_label(_("Select automatically"));
	gtk_widget_set_tooltip_text(_checkButton, _("Automatically select MIDI Controller when a CC message is received"));
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(_checkButton), TRUE);

	// Using a box to left-align checkbutton within the table cell
	GtkWidget *box = gtk_hbox_new(FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), _checkButton, FALSE, FALSE, 0);

	GtkWidget *table = gtk_table_new(3, 2, FALSE);
	gtk_table_attach(GTK_TABLE(table), gtk_label_new(_("Synth Parameter:")), 0, 1, 0, 1, GTK_FILL, GTK_FILL, 5, 5);
	gtk_table_attach(GTK_TABLE(table), _paramNameEntry,                      1, 2, 0, 1, GTK_FILL, GTK_FILL, 5, 5);
	gtk_table_attach(GTK_TABLE(table), gtk_label_new(_("MIDI Controller")),  0, 1, 1, 2, GTK_FILL, GTK_FILL, 5, 5);
	gtk_table_attach(GTK_TABLE(table), _combo,                               1, 2, 1, 2, GTK_FILL, GTK_FILL, 5, 5);
	gtk_table_attach(GTK_TABLE(table), box,                                  1, 2, 2, 3, GTK_FILL, GTK_FILL, 5, 5);

	GtkWidget *content = gtk_dialog_get_content_area (GTK_DIALOG (_dialog));
	gtk_box_pack_start(GTK_BOX(content), table, TRUE, TRUE, 0);

	_midiController->getLastControllerParam().addUpdateListener(this);
}

MIDILearnDialog::~MIDILearnDialog()
{
	_midiController->getLastControllerParam().removeUpdateListener(this);
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
	g_idle_add(MIDILearnDialog::last_active_controller_changed, this);
}

gboolean
MIDILearnDialog::last_active_controller_changed(gpointer data)
{
	MIDILearnDialog *dialog = (MIDILearnDialog *) data;
	if (!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(dialog->_checkButton)))
		return G_SOURCE_REMOVE;
	int cc = (int)dialog->_midiController->getLastControllerParam().getValue();
	gtk_combo_box_set_active (GTK_COMBO_BOX (dialog->_combo), cc + 1);
	return G_SOURCE_REMOVE;
}
