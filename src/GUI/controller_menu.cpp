/*
 *  controller_menu.cpp
 *
 *  Copyright (c) 2001-2016 Nick Dowell
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

#include "controller_menu.h"

#include "../Configuration.h"
#include "../Preset.h"

#include <glib/gi18n.h>

extern "C" void modal_midi_learn(Param param_index);

static void
show_midi_learn_dialog (GtkMenuItem *, gpointer user_data)
{
	modal_midi_learn((Param)(int)(long)user_data);
}

static void
override_item_toggled (GtkCheckMenuItem *item, gpointer user_data)
{
	Preset::setShouldIgnoreParameter((int)(long)user_data, gtk_check_menu_item_get_active (item) == TRUE);
	Configuration config = Configuration::get();
	config.ignored_parameters = Preset::getIgnoredParameterNames();
	config.save();
	return;
}

GtkWidget *controller_menu_new(int parameter)
{
	GtkWidget *item, *menu = gtk_menu_new ();

	item = gtk_menu_item_new_with_label(_("MIDI Learn..."));
	g_signal_connect(item, "activate", G_CALLBACK(show_midi_learn_dialog), (gpointer)(long)parameter);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	item = gtk_check_menu_item_new_with_label(_("Ignore Preset Value"));
	gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), Preset::shouldIgnoreParameter(parameter));
	g_signal_connect(item, "toggled", G_CALLBACK(override_item_toggled), (gpointer)(long)parameter);
	gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

	gtk_widget_show_all (menu);

	return menu;
}
