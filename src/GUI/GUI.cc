/*
 *  GUI.cc
 *
 *  Copyright (c) 2001-2016 Nick Dowell
 *  Portions of this file (c) 2003 Darrick Servis
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
 
#include "GUI.h"

#include <assert.h>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <sys/types.h>

#include <gtkmm.h>
#include <gtk/gtk.h>
#include <sigc++/bind.h>

#if defined(__linux)
#include <X11/Xlib.h>
#include <gdk/gdkx.h>
#endif

using namespace Gtk;
using std::cout;
using std::cerr;
using std::endl;
using std::ostringstream;

#include "../AudioOutput.h"
#include "../Configuration.h"
#include "../MidiController.h"
#include "../Preset.h"
#include "../Synthesizer.h"
#include "ConfigDialog.h"
#include "editor_pane.h"
#include "gui_main.h"
#include "MIDILearnDialog.h"
#include "PresetControllerView.h"

#include <glib/gi18n.h>

enum {
	evLoad,
	evCommit,
	evPresetRename,
	evPresetRenameOk,
	evPresetClear,
	evPresetExport,
	evPresetImport,
	evQuit,
	evRecDlgFileChooser,
	evRecDlgClose,
	evRecDlgRecord,
	evRecDlgPause,
	evVkeybd,
	evConfig,
	evNewInstance,
    evHelpMenuAbout,
	evHelpMenuBugReport,
	evHelpMenuOnlineDocumentation,

	evMax
};

static MIDILearnDialog *g_midiLearn = NULL;

void modal_midi_learn(int param_index) // called by editor_pane upon right-clicking a control
{
	if (g_midiLearn)
		g_midiLearn->run_modal(param_index);
}

bool
GUI::on_delete_event(GdkEventAny *)
{
	return !confirm_quit();
}

bool
GUI::confirm_quit()
{
	if (m_presetIsNotSaved) {
        GtkWidget *dialog;
        dialog = gtk_message_dialog_new_with_markup(this->gobj(),
                                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    GTK_MESSAGE_WARNING,
                                                    GTK_BUTTONS_NONE,
                                                    _("<b>Save changes before closing?</b>"));

        gtk_dialog_add_button (GTK_DIALOG (dialog), _("Close _Without Saving"), GTK_RESPONSE_NO);
        gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
        gtk_dialog_add_button (GTK_DIALOG (dialog), GTK_STOCK_SAVE, GTK_RESPONSE_YES);
        
        gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG(dialog),
                                                  _("If you don't save, changes to the current preset will be permanently lost."));
        
        gint result = gtk_dialog_run (GTK_DIALOG (dialog));
        gtk_widget_destroy (dialog);
        
        if (result == GTK_RESPONSE_CANCEL) {
            return false;
        }
        
        if (result == GTK_RESPONSE_YES) {
            preset_controller->loadPresets(); // in case another instance has changed any of the other presets
            preset_controller->commitPreset();
            preset_controller->savePresets();
        }
	}
	return true;
}

GUI::GUI(MidiController & mc, Synthesizer *synth, GenericOutput *audio)
:	m_auditionKeyDown(false)
,	m_synth(synth)
,	m_presetIsNotSaved(false)
{
	this->midi_controller = &mc;
	this->audio_out = audio;
	
	set_resizable(false);
        
	presetCV = PresetControllerView::create();

	//
	// the preset rename dialog
	//
	preset_rename.set_title(_("Rename Preset"));
	preset_rename.set_size_request( 300, 200 );
	preset_rename.set_resizable (false);
	preset_rename.get_vbox()->add( preset_rename_label );
	preset_rename_label.set_text(_("Enter new Preset Name:"));
	preset_rename.get_vbox()->add( preset_rename_entry );
	preset_rename.get_action_area()->add( preset_rename_ok );
	preset_rename_ok.add_label(_("Confirm"), 0.5, 0.5 );
	preset_rename_ok.signal_clicked().connect(sigc::bind( sigc::mem_fun(*this, &GUI::event_handler), (int) evPresetRenameOk));
	preset_rename.get_action_area()->add( preset_rename_cancel );
	preset_rename_cancel.signal_clicked().connect(mem_fun(preset_rename, &Gtk::Dialog::hide));
	preset_rename_cancel.add_label(_("Cancel"), 0.5, 0.5 );
	preset_rename.set_modal( true );
	preset_rename.set_transient_for( *this );
	
	//
	// the record dialog
	//
	record_dialog.set_title(_("Capture Output"));
	record_dialog.add( record_vbox );
	record_dialog.set_resizable (false);
	
	record_vbox.set_spacing( 10 );
	record_vbox.pack_start( record_file_frame, TRUE, TRUE, 0 );
	record_vbox.pack_start( record_buttons_hbox, TRUE, TRUE, 0 );
	record_vbox.pack_start( record_statusbar, FALSE, FALSE, 0 );
	
	record_file_frame.set_border_width( 5 );
	record_file_frame.set_label(_("output file:"));
	record_file_frame.add( record_file_hbox );
	record_file_hbox.set_border_width( 5 );
	record_file_hbox.set_spacing( 10 );
	record_file_hbox.add( record_entry );
	record_file_hbox.add( record_choose );
	record_entry.set_text( _("amsynth-out.wav") );
	record_choose.add_label( "...", 0.5, 0.5 );
	record_choose.signal_clicked().connect(bind(mem_fun(this, &GUI::event_handler),(int)evRecDlgFileChooser));
		
	record_buttons_hbox.add( record_record );
	record_buttons_hbox.add( record_pause );
	record_buttons_hbox.set_border_width( 10 );
	record_buttons_hbox.set_spacing( 10 );
	record_record.add_label( "REC", 0.5, 0.5 );
	record_pause.add_label( "STOP", 0.5, 0.5 );
	record_record.signal_clicked().connect(sigc::bind(mem_fun(*this, &GUI::event_handler),(int)evRecDlgRecord) );
	record_pause.signal_clicked().connect(sigc::bind(mem_fun(*this, &GUI::event_handler),(int)evRecDlgPause) );
	
	record_recording = false;
	record_statusbar.push (_("capture status: STOPPED"), 1);
}

Gtk::MenuBar*
GUI::create_menus	( )
{
	using namespace Gtk::Menu_Helpers;
	using namespace Gtk;
	using sigc::bind;

	Configuration & config = Configuration::get();
	
	//
	// File menu
	//
	Menu *menu_file = manage (new Menu());
	MenuList& list_file = menu_file->items ();
	
#if defined(__linux)
	// create-new-instance currently only supported on Linux
	list_file.push_back (MenuElem(_("New Instance"), sigc::bind(mem_fun(this, &GUI::event_handler),(int)evNewInstance)));
	list_file.push_back (SeparatorElem());
#endif
	
	list_file.push_back (MenuElem(_("_Open Bank..."), Gtk::AccelKey("<control>O"), mem_fun(*this, &GUI::bank_open)));
//	list_file.push_back (MenuElem("_Save Bank","<control>S", mem_fun(*this, &GUI::bank_save)));
	list_file.push_back (MenuElem(_("_Save Bank As..."), Gtk::AccelKey("<control>S"), mem_fun(*this, &GUI::bank_save_as)));
	list_file.push_back (SeparatorElem());
	list_file.push_back (MenuElem(_("Open Alternate Tuning File..."), mem_fun(*this, &GUI::scale_open)));
	list_file.push_back (MenuElem(_("Open Alternate Keyboard Map..."), mem_fun(*this, &GUI::key_map_open)));
	list_file.push_back (MenuElem(_("Reset All Tuning Settings to Default"), mem_fun(*this, &GUI::tuning_reset)));
	list_file.push_back (SeparatorElem());
	list_file.push_back (MenuElem(_("_Quit"), Gtk::AccelKey("<control>Q"), bind(mem_fun(this, &GUI::event_handler),(int)evQuit)));
	
	
	//
	// Preset menu
	//
	Menu *menu_preset = manage (new Menu());
	MenuList& list_preset = menu_preset->items ();
//	list_preset.push_back (manage (new TearoffMenuItem ()));
	list_preset.push_back (MenuElem(_("_Copy"), Gtk::AccelKey("<control>C"), mem_fun(*this, &GUI::preset_copy)));
	list_preset.push_back (MenuElem(_("_Paste"), Gtk::AccelKey("<control>V"), mem_fun(*this, &GUI::preset_paste)));
	list_preset.push_back (SeparatorElem());
	list_preset.push_back (MenuElem(_("Rename..."), sigc::bind(mem_fun(*this, &GUI::event_handler), (int)evPresetRename)));
	list_preset.push_back (MenuElem(_("Clear"), bind(mem_fun(this,&GUI::event_handler),(int)evPresetClear)));
	list_preset.push_back (SeparatorElem());
	list_preset.push_back (MenuElem(_("_Randomise"), Gtk::AccelKey("<control>R"), sigc::mem_fun(preset_controller, &PresetController::randomiseCurrentPreset)));
	list_preset.push_back (MenuElem(_("Undo"), Gtk::AccelKey("<control>Z"), sigc::mem_fun(preset_controller, &PresetController::undoChange)));
	list_preset.push_back (MenuElem(_("Redo"), Gtk::AccelKey("<control>Y"), sigc::mem_fun(preset_controller, &PresetController::redoChange)));
	list_preset.push_back (SeparatorElem());
	list_preset.push_back (MenuElem(_("Import Preset..."), bind(mem_fun(*this, &GUI::event_handler), (int)evPresetImport)));
	list_preset.push_back (MenuElem(_("Export Preset..."), bind(mem_fun(*this, &GUI::event_handler), (int)evPresetExport)));

			
	//
	// Config menu
	//
	Menu *menu_config = manage (new Menu());
	MenuList& list_config = menu_config->items ();

	//
	// Config > MIDI Channel
	//
	{
		Gtk::RadioButtonGroup grp;
		Gtk::RadioMenuItem *item = NULL;
		Gtk::Menu *menu = Gtk::manage( new Gtk::Menu );
		const int currentValue = midi_controller->get_midi_channel();
		
		for (int i=0; i<=16; i++) {
			ostringstream name; i ? name << i : name << _("All");
			item = Gtk::manage(new Gtk::RadioMenuItem(grp, name.str()));
			item->set_active((i == currentValue));
			menu->items().push_back(*item);
		}
		
		// connect the signal handlers after calling set_active to prevent altering the value.
		for (int i=0; i<16; i++) {
			menu->items()[i].signal_activate().connect( sigc::bind(mem_fun(*this, &GUI::on_midi_channel_change), i) );
		}

		menu_config->items().push_back(MenuElem(_("MIDI Channel"), *menu));
	}
	
	//
	// Config > Polyphony
	//
	{
		Gtk::RadioButtonGroup grp;
		Gtk::RadioMenuItem *item = NULL;
		Gtk::Menu *menu = Gtk::manage( new Gtk::Menu );
		const int currentValue = m_synth->getMaxNumVoices();
		
		item = Gtk::manage(new Gtk::RadioMenuItem(grp, _("Unlimited")));
		item->signal_activate().connect( sigc::bind(mem_fun(*this, &GUI::on_ployphony_change), 0, item) );
		menu->items().push_back(*item);
		
		for (int i=1; i<=16; i++) {
			ostringstream name; name << i;
			Gtk::RadioMenuItem *item = Gtk::manage(new Gtk::RadioMenuItem(grp, name.str()));
			item->set_active((i == currentValue));
			item->signal_activate().connect( sigc::bind(mem_fun(*this, &GUI::on_ployphony_change), i, item) );
			menu->items().push_back(*item);
		}
		
		menu_config->items().push_back(MenuElem(_("Max. Polyphony"), *menu));
	}

	//
	// Config > Pitch bend range
	//
	{
		Gtk::RadioButtonGroup grp;
		m_pitchBendRangeMenu = Gtk::manage( new Gtk::Menu );

		for (int i=1; i<=24; i++) {
			ostringstream name;
			name << i;
			name << _(" Semitones");
			Gtk::RadioMenuItem *item = Gtk::manage(new Gtk::RadioMenuItem(grp, name.str()));
			item->set_active(i == config.pitch_bend_range);
			item->signal_activate().connect( sigc::bind(mem_fun(*this, &GUI::on_pitch_bend_range_change), i, item) );
			m_pitchBendRangeMenu->items().push_back(*item);
		}

		m_pitchBendRangeMenu->signal_show().connect(sigc::mem_fun(*this, &GUI::on_pitch_bend_range_menu_show));

		menu_config->items().push_back(MenuElem(_("Pitch Bend Range"), *m_pitchBendRangeMenu));
	}
	
	list_config.push_back (MenuElem(_("Audio & MIDI..."), bind(mem_fun(*this, &GUI::event_handler), (int)evConfig)));
	
	
	//
	// Utils menu
	//
	Menu *menu_utils = manage (new Menu());
	MenuList& list_utils = menu_utils->items ();

	MenuItem *menu_item = manage (new MenuItem(_("Virtual Keyboard")));
	menu_item->signal_activate().connect(sigc::bind(mem_fun(*this, &GUI::event_handler),(int)evVkeybd));
	// vkeybd must exist, and we must be using ALSA MIDI
	if (config.alsa_seq_client_id == 0 || command_exists("vkeybd") != 0)
		menu_item->set_sensitive( false );
	list_utils.push_back (*menu_item);

	menu_item = manage (new MenuItem(_("Record to .wav file...")));
	menu_item->signal_activate().connect(mem_fun(record_dialog, &Gtk::Dialog::show_all));
	if (audio_out) if (!audio_out->canRecord ()) menu_item->set_sensitive (false);
	list_utils.push_back (*menu_item);

	list_utils.push_back (SeparatorElem());
	
	//
	// ALSA-MIDI sub-menu
	//
	Menu *menu_utils_midi = manage (new Menu());
	MenuList& list_utils_midi = menu_utils_midi->items ();
	
	menu_item = manage (new MenuItem("kaconnect"));
	menu_item->signal_activate().connect(sigc::bind(mem_fun(*this, &GUI::command_run),"kaconnect"));
	if (command_exists ("kaconnect") != 0) menu_item->set_sensitive( false );
	if (config.alsa_seq_client_id==0) menu_item->set_sensitive( false );
	list_utils_midi.push_back (*menu_item);
	
	menu_item = manage (new MenuItem("alsa-patch-bay"));
	menu_item->signal_activate().connect (sigc::bind(mem_fun(*this, &GUI::command_run),"alsa-patch-bay --driver alsa"));
	if (command_exists ("alsa-patch-bay") != 0) menu_item->set_sensitive( false );
	if (config.alsa_seq_client_id==0) menu_item->set_sensitive( false );
	list_utils_midi.push_back (*menu_item);
	
	list_utils.push_back (MenuElem(_("MIDI (ALSA) connections"), *menu_utils_midi));
	
	//
	// JACK sub-menu
	//
	Menu *menu_utils_jack = manage (new Menu());
	MenuList& list_utils_jack = menu_utils_jack->items ();
	
	menu_item = manage (new MenuItem("qjackconnect"));
	menu_item->signal_activate().connect (sigc::bind(mem_fun(*this, &GUI::command_run),"qjackconnect"));
	if (command_exists ("qjackconnect") != 0) menu_item->set_sensitive( false );
	if (config.current_audio_driver != "jack" && config.current_audio_driver != "JACK") menu_item->set_sensitive( false );
	list_utils_jack.push_back (*menu_item);
	
	menu_item = manage (new MenuItem("alsa-patch-bay"));
	menu_item->signal_activate().connect (sigc::bind(mem_fun(*this, &GUI::command_run),"alsa-patch-bay --driver jack"));
	if (command_exists ("alsa-patch-bay") != 0) menu_item->set_sensitive( false );
	if (config.current_audio_driver != "jack" && config.current_audio_driver != "JACK") menu_item->set_sensitive( false );
	list_utils_jack.push_back (*menu_item);
	
	list_utils.push_back (MenuElem(_("Audio (JACK) connections"), *menu_utils_jack));

	//
	// Help menu
	//
	Menu *menu_help = manage (new Menu());
	menu_help->items().push_back (MenuElem(_("About"), bind(mem_fun(this, &GUI::event_handler), (int)evHelpMenuAbout)));
	menu_help->items().push_back (MenuElem(_("Report a Bug"), bind(mem_fun(this, &GUI::event_handler), (int)evHelpMenuBugReport)));
	menu_help->items().push_back (MenuElem(_("Online Documentation"), bind(mem_fun(this, &GUI::event_handler), (int)evHelpMenuOnlineDocumentation)));

	
	//
	// Menubar
	//
	MenuBar *menu_bar = manage (new MenuBar ());
	
	MenuList& list_bar = menu_bar->items();
	list_bar.push_back (MenuElem(_("_File"), Gtk::AccelKey("<alt>F"), *menu_file));
	list_bar.push_back (MenuElem(_("_Preset"), Gtk::AccelKey("<alt>P"), *menu_preset));
	list_bar.push_back (MenuElem(_("_Config"), Gtk::AccelKey("<alt>C"), *menu_config));
	list_bar.push_back (MenuElem(_("_Utils"), Gtk::AccelKey("<alt>U"), *menu_utils));
	list_bar.push_back (MenuElem(_("_Help"), *menu_help));
    
    gchar *text = g_strdup_printf (_("Audio: %s @ %d  MIDI: %s"), config.current_audio_driver.c_str(), config.sample_rate, config.current_midi_driver.c_str());
    list_bar.push_back (MenuElem (text));
    list_bar.back().set_right_justified();
    list_bar.back().set_sensitive(false);
    g_free (text);

	return menu_bar;
}


void
adjustment_value_changed (GtkAdjustment *adjustment, gpointer data)
{
	Parameter *param = (Parameter *)data;
	param->setValue (gtk_adjustment_get_value (adjustment));
}

void
start_atomic_adjustment_value_change (GtkAdjustment *adjustment, gpointer data)
{
	UndoArgs *args = (UndoArgs *) data;
	args->presetController->pushParamChange(args->parameter->GetId(), gtk_adjustment_get_value(adjustment));
}

void 
GUI::init()
{
	Preset *preset = &(preset_controller->getCurrentPreset());
	
	for (int i=0; i<kAmsynthParameterCount; i++) {
		Parameter &param = preset->getParameter(i);
		m_adjustments[i] = (GtkAdjustment *) gtk_adjustment_new (
			param.getValue(),
			param.getMin(),
			param.getMax(),
			param.getStep(),
			0, 0);
	}
	
	Gtk::Widget *editor = Glib::wrap (editor_pane_new (m_adjustments, FALSE));
	
	// start_atomic_value_change is not registered until editor_pane_new is called
	for (int i=0; i<kAmsynthParameterCount; i++) {
		Parameter &param = preset->getParameter(i);
		m_undoArgs[i] = new UndoArgs(preset_controller, &param);
		g_signal_connect_after (G_OBJECT (m_adjustments[i]), "start_atomic_value_change",
			G_CALLBACK(start_atomic_adjustment_value_change),
			(gpointer) m_undoArgs[i] );
		gtk_signal_connect (GTK_OBJECT (m_adjustments[i]), "value_changed",
			(GtkSignalFunc) adjustment_value_changed,
			(gpointer) &param );
	}
	
	vbox.pack_start (*(create_menus ()),0,0);
	vbox.pack_start (*presetCV, false, false);
	vbox.pack_start (*editor, Gtk::PACK_EXPAND_WIDGET,0);
	add(vbox);

	show_all();
	
	//
	// show any error dialogs after entering gtk's run loop, otherwise the user
	// will see the window's controls adjust their layout after dismissing the
	// dialog (which looks ugly).
	//
	CALL_ON_GUI_THREAD( *this, &GUI::post_init );
}

void
GUI::post_init()
{
	Configuration & config = Configuration::get();
	bool bad_config = false;
	
	if (config.current_audio_driver.empty())
	{
		bad_config = true;
		MessageDialog dlg (*this, _("amsynth configuration error"), false, MESSAGE_ERROR, BUTTONS_OK, true);
		dlg.set_secondary_text(
			_("amsynth could not initialise the selected audio device.\n\n"
			"Please review the configuration and restart")
		    );
		dlg.run();
	}
	
	if (config.current_midi_driver.empty())
	{
		bad_config = true;
		MessageDialog dlg (*this, _("amsynth configuration error"), false, MESSAGE_ERROR, BUTTONS_OK, true);
		dlg.set_secondary_text(
			_("amsynth could not initialise the selected MIDI device.\n\n"
			"Please review the configuration and restart")
		    );
		dlg.run();
	}
	
	if (bad_config)
	{
		// open config dialog
		event_handler(evConfig);
		return;
	}
	
#ifdef ENABLE_REALTIME
	// show realtime warning message if necessary
	if (config.current_audio_driver_wants_realtime == 1 &&
		config.realtime == 0)
	{
		MessageDialog dlg (*this, _("amsynth could not set realtime priority"));
		dlg.set_secondary_text (_("You may experience audio buffer underruns resulting in 'clicks' in the audio.\n\nThis is most likely because the program is not SUID root.\n\nUsing the JACK audio subsystem can also help"));
		dlg.run();
	}
#endif
}

static void
open_uri(const char *uri)
{
	GError *error = NULL;
	if (!g_app_info_launch_default_for_uri(uri, NULL, &error)) {
		GtkWidget *dialog = gtk_message_dialog_new(NULL, //GTK_WINDOW(window),
												   GTK_DIALOG_DESTROY_WITH_PARENT,
												   GTK_MESSAGE_ERROR,
												   GTK_BUTTONS_OK,
												   _("Could not show link"));
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", error->message);
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
}

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

void
GUI::event_handler(const int e)
{
	switch (e)
	{
	case evLoad:
		break;

	case evCommit:
		break;

	case evPresetRename:
		preset_rename_entry.set_text(preset_controller->getCurrentPreset().getName());
		preset_rename_entry.grab_focus();
		preset_rename.show_all();
		break;
	
	case evPresetRenameOk:
		preset_controller->getCurrentPreset().setName(preset_rename_entry.get_text());
		onUpdate();
		presetCV->update();
		preset_rename.hide();
		break;
	
	case evPresetClear:
		{
			GtkWidget *dialog = gtk_message_dialog_new (
					this->gobj(),
					GTK_DIALOG_MODAL,
					GTK_MESSAGE_QUESTION,
					GTK_BUTTONS_YES_NO,
					_("Clear current preset?"));

			gtk_message_dialog_format_secondary_text (
					GTK_MESSAGE_DIALOG (dialog),
					_("Parameters will be set to default values and the name will be cleared"));

			gint result = gtk_dialog_run (GTK_DIALOG (dialog));
			if (result == GTK_RESPONSE_YES) {
				preset_controller->clearPreset();
				presetCV->update();
			}

			gtk_widget_destroy (dialog);
		}
		break;
	
	case evPresetExport:
		{
			std::string filename = file_dialog(this->gobj(), _("Export Preset"), true, NULL, NULL, (preset_controller->getCurrentPreset().getName() + ".amSynthPreset").c_str());
			if (!filename.empty()) {
				preset_controller->exportPreset(filename);
			}
		}
		break;
	
	case evPresetImport:
		{
			std::string filename = file_dialog(this->gobj(), _("Import Preset"), false, _("amsynth 1.x files"), "*.amSynthPreset", NULL);
			if (!filename.empty()) {
				preset_controller->importPreset(filename);
			}
		}
		break;
	
	case evQuit:
		if (confirm_quit()) {
			gtk_main_quit();
		}
		break;
	
	case evRecDlgFileChooser:
		{
			std::string filename = file_dialog(this->gobj(), _("Select output WAV file"), true, NULL, NULL, NULL);
			if (!filename.empty()) {
				record_entry.set_text(filename);
			}
		}
		break;
	
	case evRecDlgClose:
		audio_out->stopRecording();
		record_dialog.hide_all();
		break;
	
	case evRecDlgRecord:
		if( record_recording == false )
		{
			audio_out->setOutputFile( record_entry.get_text() );
			audio_out->startRecording();
			record_recording = true;
			record_statusbar.pop( 1 );
			record_statusbar.push (_("capture status: RECORDING"), 1);
		}
		break;
	
	case evRecDlgPause:
	    if( record_recording == true )
	    {
		    audio_out->stopRecording();
		    record_recording = false;
		    record_statusbar.pop( 1 );
		    record_statusbar.push (_("capture status: STOPPED"), 1);
	    }
	    break;
	
	case evVkeybd:
	    {
			char tmp[255] = "";
			Configuration & config = Configuration::get();
			snprintf(tmp, sizeof(tmp), "vkeybd --addr %d:0", config.alsa_seq_client_id);
			command_run(tmp);
	    }
		break;
	
	case evConfig:
	{
		ConfigDialog dlg (*this);
		dlg.run ();
		break;
	}
	
	case evNewInstance:
		spawn_new_instance();
		break;
            
    case evHelpMenuAbout: {
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
            NULL
        };
        std::string version = VERSION;
        gtk_show_about_dialog(this->gobj(),
                              "program-name", PACKAGE,
                              "logo-icon-name", PACKAGE,
                              "version", version.c_str(),
                              "authors", authors,
                              "translator-credits", "Olivier Humbert - French\nGeorg Krause - German\nPeter Körner - German",
                              "comments", _("Analogue Modelling SYNTHesizer"),
                              "website", PACKAGE_URL,
                              "copyright", _("Copyright © 2002 - 2016 Nick Dowell and contributors"),
                              NULL);
        break;
    }

	case evHelpMenuBugReport:
		open_uri(PACKAGE_BUGREPORT);
		break;

	case evHelpMenuOnlineDocumentation:
		open_uri("https://github.com/amsynth/amsynth/wiki");
		break;

	default:
		cout << "no handler for event: " << e << endl;
		break;
    }
}

GUI::~GUI()
{
	for(int i = 0; i < kAmsynthParameterCount; i++) delete m_undoArgs[i];
}

void
GUI::update()
{
	CALL_ON_GUI_THREAD( *this, &GUI::onUpdate );
}

void
GUI::onUpdate()	// called whenever the preset selection has changed
{
	update_title();
	UpdateParameterOnMainThread((Param)-1, 0); // to update the '*' in window title
    presetCV->update(); // note: PresetControllerView::update() is expensive
}

void
GUI::update_title()
{
	Configuration & config = Configuration::get();
	std::ostringstream ostr;
	ostr << "amsynth";

	if (config.jack_client_name.length() && config.jack_client_name != "amsynth") {
		ostr << ": ";
		ostr << config.jack_client_name;
	}

	ostr << ": ";
	ostr << preset_controller->getCurrPresetNumber();

	ostr << ": ";
	ostr << preset_controller->getCurrentPreset().getName();

	if (m_presetIsNotSaved) {
		ostr << " *";
	}

	set_title(ostr.str());
}

void
GUI::UpdateParameter(Param paramID, float paramValue)
{
	call_slot_on_gui_thread(
		sigc::bind( 
			sigc::mem_fun(
				*this, &GUI::UpdateParameterOnMainThread
			),
			paramID, paramValue
		)
	);
}

void
GUI::UpdateParameterOnMainThread(Param paramID, float)	// called whenever a parameter value has changed
{
	if (0 <= paramID && paramID < kAmsynthParameterCount) {
		const Parameter &param = preset_controller->getCurrentPreset().getParameter(paramID);
		gtk_adjustment_set_value (m_adjustments[paramID], param.getValue());
	}
	bool isModified = preset_controller->isCurrentPresetModified();
	if (m_presetIsNotSaved != isModified) {
		m_presetIsNotSaved = isModified;
		update_title();
	}
}

void 
GUI::run()
{
    int argc = 0;
    char **argv;

    Gtk::Main kit(&argc, &argv);
    kit.run();
}

void 
GUI::setPresetController(PresetController & p_c)
{
    preset_controller = &p_c;
    preset_controller->setUpdateListener(*this);
    presetCV->setPresetController(preset_controller);
	onUpdate();
	
	// register for notification of all parameter changes
	Preset &preset = preset_controller->getCurrentPreset();
	unsigned paramCount = preset.ParameterCount();
	for (unsigned i=0; i<paramCount; i++) {
		preset.getParameter(i).addUpdateListener(*this);
	}
	
	g_midiLearn = new MIDILearnDialog(midi_controller, preset_controller, this->gobj());
}

////////////////////////////////////////////////////////////////////////////////

void
GUI::preset_copy	( )
{
	std::string presetData = preset_controller->getCurrentPreset().toString();
	gtk_clipboard_set_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), presetData.c_str(), -1);
}

void
GUI::preset_paste	( )
{
	gtk_clipboard_request_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), GUI::preset_paste_callback, this);
}

void
GUI::preset_paste_callback(GtkClipboard *, const gchar *text, gpointer userdata)
{
	GUI *_this = reinterpret_cast<GUI *>(userdata);
	
	Preset pastedPreset;
	if (!text || !pastedPreset.fromString(std::string(text)))
		return;
	
	// enure preset has a unique name
	for (int suffixNumber = 1; _this->preset_controller->containsPresetWithName(pastedPreset.getName()); suffixNumber++) {
		std::stringstream str; str <<  pastedPreset.getName() << " " << suffixNumber;
		pastedPreset.setName(str.str());
	}
	
	_this->preset_controller->getCurrentPreset() = pastedPreset;
	_this->presetCV->update();
}

////////////////////////////////////////////////////////////////////////////////

void
GUI::bank_open		( )
{
	Configuration & config = Configuration::get();
	std::string filename = file_dialog(this->gobj(), _("Open Bank"), false, NULL, NULL, NULL);
	if (!filename.empty()) {
		preset_controller->savePresets(config.current_bank_file.c_str());
		config.current_bank_file = filename;
		preset_controller->loadPresets(config.current_bank_file.c_str());
	}
}

void
GUI::bank_save_as	( )
{
	GtkWidget *chooser = gtk_file_chooser_dialog_new (
		_("Save Bank"),
		this->gobj(),
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL);

	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (chooser), PresetController::getUserBanksDirectory().c_str());
	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (chooser), _("new.bank"));
	gtk_file_chooser_set_do_overwrite_confirmation (GTK_FILE_CHOOSER (chooser), TRUE);

	if (gtk_dialog_run (GTK_DIALOG (chooser)) == GTK_RESPONSE_ACCEPT) {
		char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (chooser));
		preset_controller->savePresets (filename);
		PresetController::rescanPresetBanks ();
		presetCV->update ();
		g_free (filename);
	}

	gtk_widget_destroy (chooser);
}

void
GUI::scale_open		( )
{
	std::string filename = file_dialog(this->gobj(), _("Open Scala (.scl) alternate tuning file"), false, _("Scala scale files"), "*.[Ss][Cc][Ll]", NULL);
	if (!filename.empty()) {
		int error = m_synth->loadTuningScale(filename.c_str());
		if (error) {
			MessageDialog msg(*this, _("Failed to load new tuning."));
			msg.set_secondary_text(_("Reading the tuning file failed for some reason. \
Make sure your file has the correct format and try again."));
			msg.run();
		}
	}
}

void
GUI::key_map_open	( )
{
	std::string filename = file_dialog(this->gobj(), _("Open alternate keyboard map (Scala .kbm format)"), false, _("Scala keyboard map files"), "*.[Kk][Bb][Mm]", NULL);
	if (!filename.empty()) {
		int error = m_synth->loadTuningKeymap(filename.c_str());
		if (error) {
			MessageDialog msg(*this, _("Failed to load new keyboard map."));
			msg.set_secondary_text(_("Reading the keyboard map file failed for some reason. \
Make sure your file has the correct format and try again."));
			msg.run();
		}
	}
}

void
GUI::tuning_reset	( )
{
	GtkWidget *dialog = gtk_message_dialog_new (
			this->gobj(),
			GTK_DIALOG_MODAL,
			GTK_MESSAGE_QUESTION,
			GTK_BUTTONS_YES_NO,
			_("Reset All Tuning Settings to Default"));

	gtk_message_dialog_format_secondary_text (
			GTK_MESSAGE_DIALOG (dialog),
			_("Discard the current scale and keyboard map?"));

	gint result = gtk_dialog_run (GTK_DIALOG (dialog));
	if (result == GTK_RESPONSE_YES) {
		m_synth->defaultTuning();
	}

	gtk_widget_destroy (dialog);
}

// returns 0 if executable was found
int
GUI::command_exists	(const char *command)
{
	gchar *argv[] = { (gchar *)"/usr/bin/which", (gchar *)command, NULL };
	gint exit_status = 0;
	if (g_spawn_sync(NULL, argv, NULL,
		(GSpawnFlags)(G_SPAWN_STDOUT_TO_DEV_NULL | G_SPAWN_STDERR_TO_DEV_NULL),
		NULL, NULL, NULL, NULL, &exit_status, NULL))
		return exit_status;
	return -1;
}

void
GUI::command_run	(const char *command)
{
	string full_command = std::string(command) + std::string(" &");
	system(full_command.c_str());
}

void
GUI::on_midi_channel_change(int value)
{
	midi_controller->set_midi_channel(value);
}

void
GUI::on_ployphony_change(int value, Gtk::RadioMenuItem *item)
{
	Configuration & config = Configuration::get();
	if (item->get_active()) {
		if (config.polyphony != value) {
			config.polyphony = value;
			config.save();
		}
		m_synth->setMaxNumVoices(value);
	}
}

void
GUI::on_pitch_bend_range_menu_show()
{
	Gtk::MenuShell::MenuList &list = m_pitchBendRangeMenu->items();
	guint index = MIN((m_synth->getPitchBendRangeSemitones() - 1), (list.size() - 1));
	Gtk::RadioMenuItem *item = (Gtk::RadioMenuItem *)&(list[index]);
	item->set_active();
}

void
GUI::on_pitch_bend_range_change(int value, Gtk::RadioMenuItem *item)
{
	Configuration & config = Configuration::get();
	if (item->get_active()) {
		if (config.pitch_bend_range != value) {
			config.pitch_bend_range = value;
			config.save();
			m_synth->setPitchBendRangeSemitones(config.pitch_bend_range);
		}
	}
}
