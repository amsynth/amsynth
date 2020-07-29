/*
 *  editor_menus.cpp
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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "editor_menus.h"

// Project includes

#include "../Configuration.h"
#include "../Preset.h"
#include "../PresetController.h"
#include "../Synthesizer.h"

#include "gui_main.h"

// External includes

#include <glib/gi18n.h>


// Function prototypes

extern "C" void modal_midi_learn(Param param_index);

static void tuning_menu_open_scl(GtkWidget *widget, Synthesizer *synth);
static void tuning_menu_open_kbm(GtkWidget *widget, Synthesizer *synth);
static void tuning_menu_reset   (GtkWidget *widget, Synthesizer *synth);


//

static void
show_midi_learn_dialog(GtkMenuItem *, gpointer user_data)
{
    modal_midi_learn((Param)(int)(long)user_data);
}

static void
override_item_toggled(GtkCheckMenuItem *item, gpointer user_data)
{
    Preset::setShouldIgnoreParameter((int)(long)user_data, gtk_check_menu_item_get_active (item) == TRUE);
    Configuration config = Configuration::get();
    config.ignored_parameters = Preset::getIgnoredParameterNames();
    config.save();
    return;
}

GtkWidget *
controller_menu_new(int parameter)
{
    GtkWidget *item, *menu = gtk_menu_new ();

    item = gtk_menu_item_new_with_label(_("Assign MIDI Controller..."));
    g_signal_connect(item, "activate", G_CALLBACK(show_midi_learn_dialog), (gpointer)(long)parameter);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    item = gtk_check_menu_item_new_with_label(_("Ignore Preset Value"));
    gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (item), Preset::shouldIgnoreParameter(parameter));
    g_signal_connect(item, "toggled", G_CALLBACK(override_item_toggled), (gpointer)(long)parameter);
    gtk_menu_shell_append (GTK_MENU_SHELL (menu), item);

    gtk_widget_show_all (menu);

    return menu;
}

//

static void
preset_menu_item_activated(GtkMenuItem *preset_item, GtkAdjustment **adjustments)
{
    gchar *bank = (gchar *)g_object_get_data(G_OBJECT(preset_item), "bank");
    size_t preset_index = (size_t)g_object_get_data(G_OBJECT(preset_item), "preset");

    PresetController presetController;
    presetController.loadPresets(bank);
    Preset &preset = presetController.getPreset((int) preset_index);
    for (unsigned int i = 0; i < kAmsynthParameterCount; i++) {
        float value = preset.getParameter(i).getValue();
        gtk_adjustment_set_value (adjustments[i], value);
    }
}

static GtkWidget *
presets_menu_new(GtkAdjustment **adjustments)
{
    char text[64];

    GtkWidget *presets_menu = gtk_menu_new ();

    for (auto &bank : PresetController::getPresetBanks()) {
        snprintf(text, sizeof(text), "[%s] %s", bank.read_only ? _("F") : _("U"), bank.name.c_str());
        GtkWidget *bank_item = gtk_menu_item_new_with_label(text);
        gtk_menu_shell_append(GTK_MENU_SHELL(presets_menu), bank_item);

        GtkWidget *bank_menu = gtk_menu_new ();
        gtk_menu_item_set_submenu(GTK_MENU_ITEM(bank_item), bank_menu);

        PresetController presetController;
        presetController.loadPresets(bank.file_path.c_str());
        for (gint i = 0; i < PresetController::kNumPresets; i++) {
            snprintf(text, sizeof(text), "%d: %s", i, presetController.getPreset(i).getName().c_str());
            GtkWidget *preset_item = gtk_menu_item_new_with_label(text);
            g_signal_connect(preset_item, "activate", G_CALLBACK(preset_menu_item_activated), adjustments);
            g_object_set_data_full(G_OBJECT(preset_item), "bank", g_strdup(bank.file_path.c_str()), g_free);
            g_object_set_data_full(G_OBJECT(preset_item), "preset", (void *)(size_t)i, nullptr);
            gtk_menu_shell_append(GTK_MENU_SHELL(bank_menu), preset_item);
        }
    }

    gtk_widget_show_all (presets_menu);
    return presets_menu;
}

//

static void
add_menu_item(GtkWidget *menu, const gchar *label, GCallback callback, gpointer data)
{
    GtkWidget *item = gtk_menu_item_new_with_label(label);
    g_signal_connect(item, "activate", callback, data);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
}

static void
add_menu_item(GtkWidget *menu, const gchar *label, GtkWidget *submenu)
{
    GtkWidget *item = gtk_menu_item_new_with_label(label);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
}

GtkWidget *
editor_menu_new(void *synth, GtkAdjustment **adjustments)
{
    GtkWidget *presets_menu = presets_menu_new(adjustments);
    if (!synth)
        return presets_menu;
    
    GtkWidget *menu = gtk_menu_new();
    
    add_menu_item(menu, _("Preset"), presets_menu);
    
    GtkWidget *item = gtk_menu_item_new_with_label(_("Tuning"));
    GtkWidget *submenu = gtk_menu_new();
    add_menu_item(submenu, _("Open Alternate Tuning File..."),          G_CALLBACK(tuning_menu_open_scl), synth);
    add_menu_item(submenu, _("Open Alternate Keyboard Map..."),         G_CALLBACK(tuning_menu_open_kbm), synth);
    add_menu_item(submenu, _("Reset All Tuning Settings to Default"),   G_CALLBACK(tuning_menu_reset),    synth);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
    
    gtk_widget_show_all(menu);
    return menu;
}

//

static GtkWidget *
file_open_dialog(GtkWindow *parent, const gchar *title, const gchar *filter_name, const gchar *filter_pattern)
{
    GtkWidget *dialog = gtk_file_chooser_dialog_new(
            title, parent,
            GTK_FILE_CHOOSER_ACTION_OPEN,
            GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
            GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
            NULL);
    
    GtkFileFilter *filter = gtk_file_filter_new();
    gtk_file_filter_set_name(filter, filter_name);
    gtk_file_filter_add_pattern(filter, filter_pattern);
    gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);
    
    return dialog;
}

static void tuning_menu_open_scl(GtkWidget *widget, Synthesizer *synth)
{
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(widget));
    GtkWidget *dialog = file_open_dialog(parent,
            _("Open Scala (.scl) alternate tuning file"),
            _("Scala scale files"), "*.[Ss][Cc][Ll]");
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (synth->loadTuningScale(filename) != 0) {
            ShowModalErrorMessage(
                    _("Failed to load new tuning."),
                    _("Reading the tuning file failed for some reason.\n"
                      "Make sure your file has the correct format and try again."));
        }
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

static void tuning_menu_open_kbm(GtkWidget *widget, Synthesizer *synth)
{
    GtkWindow *parent = GTK_WINDOW(gtk_widget_get_toplevel(widget));
    GtkWidget *dialog = file_open_dialog(parent,
            _("Open alternate keyboard map (Scala .kbm format)"),
            _("Scala keyboard map files"), "*.[Kk][Bb][Mm]");
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if (synth->loadTuningKeymap(filename) != 0) {
            ShowModalErrorMessage(
                    _("Failed to load new keyboard map."),
                    _("Reading the keyboard map file failed for some reason.\n"
                      "Make sure your file has the correct format and try again."));
        }
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

static void tuning_menu_reset(GtkWidget *widget, Synthesizer *synth)
{
    synth->loadTuningKeymap(nullptr);
    synth->loadTuningScale(nullptr);
}
