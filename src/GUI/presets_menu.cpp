/*
 *  presets_menu.cpp
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

#include "presets_menu.h"

#include "../PresetController.h"

static void preset_menu_item_activated(GtkMenuItem *preset_item, GtkAdjustment **adjustments)
{
	gchar *bank = (gchar *)g_object_get_data(G_OBJECT(preset_item), "bank");
	size_t preset_index = (size_t)g_object_get_data(G_OBJECT(preset_item), "preset");

	PresetController presetController;
	presetController.loadPresets(bank);
	Preset &preset = presetController.getPreset(preset_index);
	for (unsigned int i = 0; i < kAmsynthParameterCount; i++) {
		float value = preset.getParameter(i).getValue();
		gtk_adjustment_set_value (adjustments[i], value);
	}
}

GtkWidget *presets_menu_new(GtkAdjustment **adjustments)
{
	char text[64];

	GtkWidget *presets_menu = gtk_menu_new ();

	const std::vector<BankInfo> banks = PresetController::getPresetBanks();

	for (size_t b=0; b<banks.size(); b++) {
		snprintf(text, sizeof(text), "[%s] %s", banks[b].read_only ? "F" : "U", banks[b].name.c_str());
		GtkWidget *bank_item = gtk_menu_item_new_with_label(text);
		gtk_menu_shell_append(GTK_MENU_SHELL(presets_menu), bank_item);

		GtkWidget *bank_menu = gtk_menu_new ();
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(bank_item), bank_menu);

		PresetController presetController;
		presetController.loadPresets(banks[b].file_path.c_str());
		for (gint i = 0; i < PresetController::kNumPresets; i++) {
			snprintf(text, sizeof(text), "%d: %s", i, presetController.getPreset(i).getName().c_str());
			GtkWidget *preset_item = gtk_menu_item_new_with_label(text);
			g_signal_connect(preset_item, "activate", G_CALLBACK(preset_menu_item_activated), adjustments);
			g_object_set_data_full(G_OBJECT(preset_item), "bank", g_strdup(banks[b].file_path.c_str()), g_free);
			g_object_set_data_full(G_OBJECT(preset_item), "preset", (void *)(size_t)i, NULL);
			gtk_menu_shell_append(GTK_MENU_SHELL(bank_menu), preset_item);
		}
	}

	gtk_widget_show_all (presets_menu);
	return presets_menu;
}
