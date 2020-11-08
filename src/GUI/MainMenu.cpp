/*
 *  MainMenu.cpp
 *
 *  Copyright(c) 2001-2020 Nick Dowell
 *
 *  This file is part of amsynth.
 *
 *  amsynth is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include "MainMenu.h"

#include "../Configuration.h"
#include "../MidiController.h"
#include "../Synthesizer.h"

#include "ConfigDialog.h"
#include "gui_main.h"

#include <glib/gi18n.h>


////

#define ACCEL_NONE NULL
#define ALT "<Alt>"
#define CTRL "<Ctrl>"
#define SHIFT "<Shift>"

static GtkWidget *
new_menu_item(GtkWidget *menu, GtkAccelGroup *accelGroup, const gchar *accelerator, const gchar *label)
{
	GtkWidget *item = gtk_menu_item_new_with_mnemonic(label);
	if (accelerator) {
		guint key;
		GdkModifierType mods;
		gtk_accelerator_parse(accelerator, &key, &mods);
		gtk_widget_add_accelerator(item, "activate", accelGroup, key, mods, GTK_ACCEL_VISIBLE);
	}
	return item;
}

static GtkWidget *
add_menu_item(GtkWidget *menu, GtkAccelGroup *accelGroup, const gchar *accelerator, const gchar *label, GCallback callback, gpointer data)
{
	GtkWidget *item = new_menu_item(menu, accelGroup, accelerator, label);
	g_signal_connect(item, "activate", callback, data);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	return item;
}

static GtkWidget *
add_menu_item(GtkWidget *menu, GtkAccelGroup *accelGroup, const gchar *accelerator, const gchar *label, GtkWidget *submenu)
{
	GtkWidget *item = new_menu_item(menu, accelGroup, accelerator, label);
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item), submenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
	return item;
}

static void
add_separator(GtkWidget *menu)
{
	GtkWidget *item = gtk_separator_menu_item_new();
	gtk_menu_shell_append(GTK_MENU_SHELL(menu), item);
}

////

static std::string
file_dialog(GtkWindow *window, const char *name, bool save, const char *filter_name, const char *filter_pattern, const char *initial_filename)
{
	GtkWidget *dialog = gtk_file_chooser_dialog_new(
			name,
			window,
			save ? GTK_FILE_CHOOSER_ACTION_SAVE : GTK_FILE_CHOOSER_ACTION_OPEN,
			GTK_STOCK_CANCEL,
			GTK_RESPONSE_CANCEL,
			save ? GTK_STOCK_SAVE : GTK_STOCK_OPEN,
			GTK_RESPONSE_ACCEPT,
			NULL);

	if (filter_name && filter_pattern) {
		GtkFileFilter *filter = gtk_file_filter_new();
		gtk_file_filter_set_name(filter, filter_name);
		gtk_file_filter_add_pattern(filter, filter_pattern);
		gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);
	}

	if (initial_filename) {
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), initial_filename);
	}

	std::string result;

	gint response = gtk_dialog_run(GTK_DIALOG(dialog));
	if (response == GTK_RESPONSE_ACCEPT) {
		char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		if (filename) {
			result = std::string(filename);
		}
		g_free(filename);
	}

	gtk_widget_destroy(dialog);

	return result;
}

////

struct FileMenu
{
	static GtkWidget * create(GtkWidget *window, GtkAccelGroup *accelGroup, Synthesizer *synthesizer)
	{
		GtkWidget *menu = gtk_menu_new();
		gtk_menu_set_accel_group(GTK_MENU(menu), accelGroup);

#if defined(__linux)
		add_menu_item(menu, accelGroup, SHIFT CTRL "N", _("New Instance"), G_CALLBACK(FileMenu::newInstance), nullptr);
		add_separator(menu);
#endif

		add_menu_item(menu, accelGroup,       CTRL "O", _("_Open Bank..."), G_CALLBACK(FileMenu::openBank), synthesizer);
		add_menu_item(menu, accelGroup, SHIFT CTRL "S", _("_Save Bank As..."), G_CALLBACK(FileMenu::saveBankAs), synthesizer);
		add_separator(menu);

		add_menu_item(menu, accelGroup, ACCEL_NONE, _("Open Alternate Tuning File..."), G_CALLBACK(FileMenu::openScaleFile), synthesizer);
		add_menu_item(menu, accelGroup, ACCEL_NONE, _("Open Alternate Keyboard Map..."), G_CALLBACK(FileMenu::openKeyboardMap), synthesizer);
		add_menu_item(menu, accelGroup, ACCEL_NONE, _("Reset All Tuning Settings to Default"), G_CALLBACK(FileMenu::resetTuning), synthesizer);
		add_separator(menu);

		add_menu_item(menu, accelGroup, CTRL "Q", _("_Quit"), G_CALLBACK(FileMenu::quit), window);

		return menu;
	}

#if defined(__linux)
	static void newInstance(GtkWidget *widget, gpointer data)
	{
		spawn_new_instance();
	}
#endif

	static void openBank(GtkWidget *widget, Synthesizer *synthesizer)
	{
		Configuration & config = Configuration::get();
		std::string filename = file_dialog(nullptr, _("Open Bank"), false, nullptr, nullptr, nullptr);
		if (!filename.empty()) {
			synthesizer->getPresetController()->savePresets(config.current_bank_file.c_str());
			config.current_bank_file = filename;
			synthesizer->getPresetController()->loadPresets(config.current_bank_file.c_str());
		}
	}

	static void saveBankAs(GtkWidget *widget, Synthesizer *synthesizer)
	{
		GtkWidget *chooser = gtk_file_chooser_dialog_new(
				_("Save Bank"), nullptr,
				GTK_FILE_CHOOSER_ACTION_SAVE,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				NULL);

		std::string directory = PresetController::getUserBanksDirectory();
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(chooser), directory.c_str());
		gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(chooser), _("new.bank"));
		gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER(chooser), TRUE);

		if (gtk_dialog_run(GTK_DIALOG(chooser)) == GTK_RESPONSE_ACCEPT) {
			char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(chooser));
			synthesizer->getPresetController()->savePresets(filename);
			synthesizer->getPresetController()->notify();
			PresetController::rescanPresetBanks();
			g_free(filename);
		}

		gtk_widget_destroy(chooser);
	}

	static void openScaleFile(GtkWidget *widget, Synthesizer *synthesizer)
	{
		std::string filename = file_dialog(nullptr, _("Open Scala (.scl) alternate tuning file"), false, _("Scala scale files"), "*.[Ss][Cc][Ll]", nullptr);
		if (!filename.empty()) {
			int error = synthesizer->loadTuningScale(filename.c_str());
			if (error) {
				ShowModalErrorMessage(_("Failed to load new tuning."),
									  _("Reading the tuning file failed for some reason.\n"
										"Make sure your file has the correct format and try again."));
			}
		}
	}

	static void openKeyboardMap(GtkWidget *widget, Synthesizer *synthesizer)
	{
		std::string filename = file_dialog(nullptr, _("Open alternate keyboard map (Scala .kbm format)"), false, _("Scala keyboard map files"), "*.[Kk][Bb][Mm]", nullptr);
		if (!filename.empty()) {
			int error = synthesizer->loadTuningKeymap(filename.c_str());
			if (error) {
				ShowModalErrorMessage(_("Failed to load new keyboard map."),
									  _("Reading the keyboard map file failed for some reason.\n"
										"Make sure your file has the correct format and try again."));
			}
		}
	}

	static void resetTuning(GtkWidget *widget, Synthesizer *synthesizer)
	{
		GtkWidget *dialog = gtk_message_dialog_new(
				nullptr,
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_YES_NO,
				"%s", _("Reset All Tuning Settings to Default"));

		gtk_message_dialog_format_secondary_text(
				GTK_MESSAGE_DIALOG(dialog),
				"%s", _("Discard the current scale and keyboard map?"));

		gint result = gtk_dialog_run(GTK_DIALOG(dialog));
		if (result == GTK_RESPONSE_YES) {
			synthesizer->loadTuningKeymap(nullptr);
			synthesizer->loadTuningScale(nullptr);
		}

		gtk_widget_destroy(dialog);
	}

	static void quit(GtkWidget *widget, GtkWidget *window)
	{
		gboolean val = FALSE;
		GdkEvent *event = gtk_get_current_event();
		g_signal_emit_by_name(G_OBJECT(window), "delete-event", event, &val);
		gdk_event_free(event);

		if (!val) {
			gtk_widget_destroy(window);
		}
	}
};

////

struct PresetMenu
{
	static GtkWidget * create(GtkWidget *window, GtkAccelGroup *accelGroup, Synthesizer *synthesizer)
	{
		GtkWidget *menu = gtk_menu_new();
		gtk_menu_set_accel_group(GTK_MENU(menu), accelGroup);

		add_menu_item(menu, accelGroup, CTRL "C", _("_Copy"),  G_CALLBACK(PresetMenu::copy), synthesizer);
		add_menu_item(menu, accelGroup, CTRL "V", _("_Paste"), G_CALLBACK(PresetMenu::paste), synthesizer);
		add_separator(menu);

		add_menu_item(menu, accelGroup, SHIFT CTRL "R", _("Rename..."), G_CALLBACK(PresetMenu::rename), synthesizer);
		add_menu_item(menu, accelGroup,       CTRL "K", _("Clear"),     G_CALLBACK(PresetMenu::clear), synthesizer);
		add_separator(menu);

		add_menu_item(menu, accelGroup, CTRL "R", _("_Randomise"), G_CALLBACK(PresetMenu::randomise), synthesizer);
		add_menu_item(menu, accelGroup, CTRL "Z", _("Undo"),       G_CALLBACK(PresetMenu::undo), synthesizer);
		add_menu_item(menu, accelGroup, CTRL "Y", _("Redo"),       G_CALLBACK(PresetMenu::redo), synthesizer);
		add_separator(menu);

		add_menu_item(menu, accelGroup, ACCEL_NONE, _("Import Preset..."), G_CALLBACK(PresetMenu::importPreset), synthesizer);
		add_menu_item(menu, accelGroup, ACCEL_NONE, _("Export Preset..."), G_CALLBACK(PresetMenu::exportPreset), synthesizer);

		return menu;
	}

	static void copy(GtkWidget *widget, Synthesizer *synthesizer)
	{
		std::string presetData = synthesizer->getPresetController()->getCurrentPreset().toString();
		gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), presetData.c_str(), -1);
	}

	static void paste(GtkWidget *widget, Synthesizer *synthesizer)
	{
		gtk_clipboard_request_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), PresetMenu::pasteCallback, synthesizer);
	}

	static void pasteCallback(GtkClipboard *, const gchar *text, gpointer data)
	{
		Synthesizer *synthesizer =(Synthesizer *)data;
		PresetController *presetController = synthesizer->getPresetController();

		Preset pastedPreset;
		if (!text || !pastedPreset.fromString(std::string(text)))
			return;

		// ensure preset has a unique name
		for(int suffixNumber = 1; presetController->containsPresetWithName(pastedPreset.getName()); suffixNumber++) {
			std::stringstream str; str <<  pastedPreset.getName() << " " << suffixNumber;
			pastedPreset.setName(str.str());
		}

		presetController->getCurrentPreset() = pastedPreset;
		presetController->notify();
	}

	static void rename(GtkWidget *widget, Synthesizer *synthesizer)
	{
		GtkWidget *dialog = gtk_dialog_new_with_buttons(
				_("Rename Preset"), nullptr, GTK_DIALOG_MODAL,
				GTK_STOCK_OK, GTK_RESPONSE_ACCEPT,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				NULL);

		GtkWidget *content = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
		GtkBox *vbox = GTK_BOX(content);

		gtk_box_pack_start(vbox, gtk_label_new(_("Enter new Preset Name:")), TRUE, 0, 0);

		GtkWidget *entry = gtk_entry_new();
		gtk_entry_set_text(GTK_ENTRY(entry), synthesizer->getPresetController()->getCurrentPreset().getName().c_str());
		gtk_box_pack_start(vbox, entry, TRUE, 0, 0);

		gtk_widget_show_all(dialog);

		gint response = gtk_dialog_run(GTK_DIALOG(dialog));

		if (response == GTK_RESPONSE_ACCEPT) {
			const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
			synthesizer->getPresetController()->getCurrentPreset().setName(text);
			synthesizer->getPresetController()->notify();
		}

		gtk_widget_destroy(dialog);
	}

	static void clear(GtkWidget *widget, Synthesizer *synthesizer)
	{
		GtkWidget *dialog = gtk_message_dialog_new(
				nullptr,
				GTK_DIALOG_MODAL,
				GTK_MESSAGE_QUESTION,
				GTK_BUTTONS_YES_NO,
				"%s", _("Clear current preset?"));

		gtk_message_dialog_format_secondary_text(
				GTK_MESSAGE_DIALOG(dialog),
				"%s", _("Parameters will be set to default values and the name will be cleared"));

		gint result = gtk_dialog_run(GTK_DIALOG(dialog));
		if (result == GTK_RESPONSE_YES) {
			synthesizer->getPresetController()->clearPreset();
		}

		gtk_widget_destroy(dialog);
	}

	static void randomise(GtkWidget *widget, Synthesizer *synthesizer)
	{
		synthesizer->getPresetController()->randomiseCurrentPreset();
	}

	static void undo(GtkWidget *widget, Synthesizer *synthesizer)
	{
		synthesizer->getPresetController()->undoChange();
	}

	static void redo(GtkWidget *widget, Synthesizer *synthesizer)
	{
		synthesizer->getPresetController()->redoChange();
	}

	static void importPreset(GtkWidget *widget, Synthesizer *synthesizer)
	{
		std::string filename = file_dialog(nullptr, _("Import Preset"), false, _("amsynth 1.x files"), "*.amSynthPreset", nullptr);
		if (!filename.empty()) {
			synthesizer->getPresetController()->importPreset(filename);
		}
	}

	static void exportPreset(GtkWidget *widget, Synthesizer *synthesizer)
	{
		std::string filename = synthesizer->getPresetController()->getCurrentPreset().getName() + ".amSynthPreset";
		filename = file_dialog(nullptr, _("Export Preset"), true, nullptr, nullptr, filename.c_str());
		if (!filename.empty()) {
			synthesizer->getPresetController()->exportPreset(filename);
		}
	}
};

////

struct ConfigMenu
{
	static GtkWidget * create(GtkWidget *window, GtkAccelGroup *accelGroup, Synthesizer *synthesizer)
	{
		GtkWidget *menu = gtk_menu_new();

		GtkWidget *channelMenu = gtk_menu_new();
		{
			const int currentValue = synthesizer->getMidiChannel();

			GSList *group = nullptr;
			for (int i = 0; i <= 16; i++) {
				std::ostringstream name; i ? name << i : name << _("All");
				GtkWidget *item = gtk_radio_menu_item_new_with_label(group, name.str().c_str());
				group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), i == currentValue);
				g_signal_connect(item, "activate", G_CALLBACK(ConfigMenu::midiChannelChange), synthesizer);
				gtk_menu_shell_append(GTK_MENU_SHELL(channelMenu), item);
			}
		}
		add_menu_item(menu, nullptr, ACCEL_NONE, _("MIDI Channel"), channelMenu);

		//

		GtkWidget *polyphonyMenu = gtk_menu_new();
		{
			const int currentValue = synthesizer->getMaxNumVoices();

			GSList *group = nullptr;
			for (int i = 0; i <= 16; i++) {
				std::ostringstream name; i ? name << i : name << _("Unlimited");
				GtkWidget *item = gtk_radio_menu_item_new_with_label(group, name.str().c_str());
				group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), i == currentValue);
				g_signal_connect(item, "activate", G_CALLBACK(ConfigMenu::polyphonyChange), synthesizer);
				gtk_menu_shell_append(GTK_MENU_SHELL(polyphonyMenu), item);
			}
		}
		add_menu_item(menu, nullptr, ACCEL_NONE, _("Max. Polyphony"), polyphonyMenu);

		//

		GtkWidget *pitchBendRange = gtk_menu_new();
		{
			const int currentValue = Configuration::get().pitch_bend_range;

			GSList *group = nullptr;
			for (int i = 1; i <= 24; i++) {
				std::ostringstream name; name << i; name << _(" Semitones");
				GtkWidget *item = gtk_radio_menu_item_new_with_label(group, name.str().c_str());
				group = gtk_radio_menu_item_get_group(GTK_RADIO_MENU_ITEM(item));
				gtk_check_menu_item_set_active(GTK_CHECK_MENU_ITEM(item), i == currentValue);
				g_signal_connect(item, "activate", G_CALLBACK(ConfigMenu::pitchBendRangeChange), synthesizer);
				gtk_menu_shell_append(GTK_MENU_SHELL(pitchBendRange), item);
			}
		}
		add_menu_item(menu, nullptr, ACCEL_NONE, _("Pitch Bend Range"), pitchBendRange);

		add_menu_item(menu, nullptr, ACCEL_NONE, _("Audio & MIDI..."), G_CALLBACK(ConfigMenu::showConfigDialog), nullptr);

		return menu;
	}

	static void midiChannelChange(GtkWidget *widget, Synthesizer *synthesizer)
	{
		if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget))) {
			return;
		}
		GtkWidget *menu = gtk_widget_get_parent(widget);
		GList *list = gtk_container_get_children(GTK_CONTAINER(menu));
		int index = g_list_index(list, widget);
		g_list_free(list);

		int newValue = index;
		int currentValue = synthesizer->getMidiChannel();
		if (currentValue != newValue) {
			Configuration::get().midi_channel = newValue;
			Configuration::get().save();
			synthesizer->setMidiChannel(newValue);
		}
	}

	static void polyphonyChange(GtkWidget *widget, Synthesizer *synthesizer)
	{
		if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget))) {
			return;
		}
		GtkWidget *menu = gtk_widget_get_parent(widget);
		GList *list = gtk_container_get_children(GTK_CONTAINER(menu));
		int index = g_list_index(list, widget);
		g_list_free(list);

		int value = index;
		if (Configuration::get().polyphony != value) {
			Configuration::get().polyphony = value;
			Configuration::get().save();
			synthesizer->setMaxNumVoices(index);
		}
	}

	static void pitchBendRangeChange(GtkWidget *widget, Synthesizer *synthesizer)
	{
		if (!gtk_check_menu_item_get_active(GTK_CHECK_MENU_ITEM(widget))) {
			return;
		}
		GtkWidget *menu = gtk_widget_get_parent(widget);
		GList *list = gtk_container_get_children(GTK_CONTAINER(menu));
		int index = g_list_index(list, widget);
		g_list_free(list);

		int value = index + 1;
		if (Configuration::get().pitch_bend_range != value) {
			Configuration::get().pitch_bend_range = value;
			Configuration::get().save();
			synthesizer->setPitchBendRangeSemitones(value);
		}
	}

	static void showConfigDialog(GtkWidget *widget, gpointer data)
	{
		config_dialog_run(nullptr);
	}
};

////

struct UtilsMenu
{
	static GtkWidget * create(GtkWidget *window, GtkAccelGroup *accelGroup)
	{
		GtkWidget *menu = gtk_menu_new();

		bool alsaMIDI = Configuration::get().alsa_seq_client_id != 0;

		GtkWidget *keyboards = gtk_menu_new();
		{
			// @translators: This is an application name and its abbreviation
			add_item(keyboards, _("Virtual Keyboard (vkeybd)"),
					 alsaMIDI && commandExists("vkeybd"),
					 G_CALLBACK(UtilsMenu::vkeybd), nullptr);

			// @translators: This is an application name and its abbreviation
			add_item(keyboards, _("Virtual MIDI Piano Keyboard (VMPK)"),
					 commandExists("vmpk"),
					 G_CALLBACK(UtilsMenu::runCommand), (gpointer) "vmpk");
		}
		add_menu_item(menu, nullptr, ACCEL_NONE, _("Virtual Keyboards"), keyboards);

		//

		GtkWidget *alsaMenu = gtk_menu_new();
		{
			add_item(alsaMenu, "kaconnect",
					 alsaMIDI && commandExists("kaconnect"),
					 G_CALLBACK(UtilsMenu::runCommand), (gpointer) "kaconnect");

			add_item(alsaMenu, "alsa-patch-bay",
					 alsaMIDI && commandExists("alsa-patch-bay"),
					 G_CALLBACK(UtilsMenu::runCommand), (gpointer) "alsa-patch-bay --driver alsa");

			add_item(alsaMenu, "Patchage",
					 alsaMIDI && commandExists("patchage"),
					 G_CALLBACK(UtilsMenu::runCommand), (gpointer) "patchage --no-jack");
		}
		add_menu_item(menu, nullptr, ACCEL_NONE, _("MIDI (ALSA) connections"), alsaMenu);

		//

		bool jackAudio = strcasecmp("jack", Configuration::get().audio_driver.c_str()) == 0;

		GtkWidget *jackMenu = gtk_menu_new();
		{
			add_item(jackMenu, "qjackconnect",
					 jackAudio && commandExists("qjackconnect"),
					 G_CALLBACK(UtilsMenu::runCommand), (gpointer) "qjackconnect");

			add_item(jackMenu, "alsa-patch-bay",
					 jackAudio && commandExists("alsa-patch-bay"),
					 G_CALLBACK(UtilsMenu::runCommand), (gpointer) "alsa-patch-bay --driver jack");

			add_item(jackMenu, "Patchage",
					 jackAudio && commandExists("patchage"),
					 G_CALLBACK(UtilsMenu::runCommand), (gpointer) "patchage --no-alsa");
		}
		add_menu_item(menu, nullptr, ACCEL_NONE, _("Audio (JACK) connections"), jackMenu);

		return menu;
	}

	static void add_item(GtkWidget *menu, const gchar *label, bool enable, GCallback callback, gpointer callbackData)
	{
		GtkWidget *item = add_menu_item(menu, nullptr, ACCEL_NONE, label, callback, callbackData);
		gtk_widget_set_sensitive(item, enable);
	}

	static void vkeybd(GtkWidget *widget, gpointer data)
	{
		char tmp[255] = "";
		Configuration & config = Configuration::get();
		snprintf(tmp, sizeof(tmp), "vkeybd --addr %d:0", config.alsa_seq_client_id);
		runCommand(widget, tmp);
	}

	static void runCommand(GtkWidget *widget, const char *command)
	{
		std::string cmd = std::string(command) + std::string(" &");
		system(cmd.c_str());
	}

	static bool commandExists(const char *command)
	{
		gchar *argv[] = { (gchar *)"/usr/bin/which", (gchar *)command, nullptr };
		gint exit_status = 0;
		if (g_spawn_sync(nullptr, argv, nullptr,
						 (GSpawnFlags)(G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL),
						 nullptr, nullptr, nullptr, nullptr, &exit_status, nullptr))
			return exit_status == 0;
		return false;
	}
};

////

struct HelpMenu
{
	static GtkWidget * create(GtkWidget *window, GtkAccelGroup *accelGroup)
	{
		GtkWidget *menu = gtk_menu_new();

		add_menu_item(menu, accelGroup, ACCEL_NONE, _("About"), G_CALLBACK(HelpMenu::about), window);
		add_menu_item(menu, accelGroup, ACCEL_NONE, _("Report a Bug"), G_CALLBACK(HelpMenu::openUri), (gpointer)PACKAGE_BUGREPORT);
		add_menu_item(menu, accelGroup, "F1", _("Online Documentation"), G_CALLBACK(HelpMenu::openUri), (gpointer)"https://github.com/amsynth/amsynth/wiki");

		return menu;
	}

	static void about(GtkWidget *widget, GtkWidget *window)
	{
		const char *authors[] = {
				"Nick Dowell",
				"Brian",
				"Karsten Wiese",
				"Jezar at dreampoint",
				"Sebastien Cevey",
				"Taybin Rutkin",
				"Bob Ham",
				"Darrick Servis",
				"Johan Martinsson",
				"Andy Ryan",
				"Chris Cannam",
				"Paul Winkler",
				"Adam Sampson",
				"Martin Tarenskeen",
				"Adrian Knoth",
				"Samuli Suominen",
				nullptr
		};
		std::string version = VERSION;
		gtk_show_about_dialog(
				GTK_WINDOW(window),
				"program-name", PACKAGE,
				"logo-icon-name", PACKAGE,
				"version", version.c_str(),
				"authors", authors,
				"translator-credits", "Olivier Humbert - French\nGeorg Krause - German\nPeter Körner - German",
				"comments", _("Analog Modelling SYNTHesizer"),
				"website", PACKAGE_URL,
				"copyright", _("Copyright © 2002 - 2020 Nick Dowell and contributors"),
				NULL);
	}

	static void openUri(GtkWidget *widget, const char *uri)
	{
		GError *error = nullptr;
		if (!g_app_info_launch_default_for_uri(uri, nullptr, &error)) {
			ShowModalErrorMessage(_("Could not show link"));
		}
	}
};

////

void main_menu_init(GtkWidget *window, GtkAccelGroup *accelGroup, GtkMenuBar *menuBar, Synthesizer *synthesizer)
{
	GtkWidget *menu = (GtkWidget *)menuBar;

	add_menu_item(menu, accelGroup, ALT "F", _("_File"),   FileMenu::create(window, accelGroup, synthesizer));
	add_menu_item(menu, accelGroup, ALT "P", _("_Preset"), PresetMenu::create(window, accelGroup, synthesizer));
	add_menu_item(menu, accelGroup, ALT "C", _("_Config"), ConfigMenu::create(window, accelGroup, synthesizer));
	add_menu_item(menu, accelGroup, ALT "U", _("_Utils"),  UtilsMenu::create(window, accelGroup));
	add_menu_item(menu, accelGroup, ALT "H", _("_Help"),   HelpMenu::create(window, accelGroup));
}
