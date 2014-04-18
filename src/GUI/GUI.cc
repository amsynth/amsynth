/*
 *  GUI.cc
 *
 *  Copyright (c) 2001-2012 Nick Dowell
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
 
#include "GUI.h"

#include <assert.h>
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

#include "../AudioOutput.h"
#include "../MidiController.h"
#include "../Preset.h"
#include "../VoiceAllocationUnit.h"

#include "../../config.h"
#include "amsynth_logo.h"
#include "ConfigDialog.h"
#include "MIDILearnDialog.h"
#include "PresetControllerView.h"

#include "gui_main.h"
#include "editor_pane.h"

enum {
	evLoad,
	evCommit,
	evPresetRename,
	evPresetRenameOk,
	evPresetDelete,
	evPresetExport,
	evPresetImport,
	evQuit,
	evRecDlgFileChooser,
	evRecDlgClose,
	evRecDlgRecord,
	evRecDlgPause,
	evVkeybd,
	evMidiSend,
	evConfig,
	evNewInstance,
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

int
GUI::delete_event_impl(GdkEventAny *)
{
	if (m_presetIsNotSaved) {
		MessageDialog dlg (*this,
			"Really quit amsynth?\n"
			"\n"
			"You will lose any changes\n"
			"which you haven't explicitly committed",
			false, MESSAGE_QUESTION, BUTTONS_YES_NO, true);
		if (RESPONSE_YES != dlg.run())
			return false;
	}
	hide_all();
	return true;
}

GUI::GUI( Config & config_in, MidiController & mc, VoiceAllocationUnit & vau_in, GenericOutput *audio )
:	m_auditionKeyDown(false)
#if ENABLE_MIDIKEYS
,	m_vkeybdOctave(4)
,	m_vkeybdIsActive(false)
,	m_vkeybdState(128)
#endif
{
	this->config = &config_in;
	this->midi_controller = &mc;
	this->vau = &vau_in;
	this->audio_out = audio;
	
	set_resizable(false);
        
	active_param = 0;
	
	
	presetCV = PresetControllerView::create(this->vau);

	//
	// the preset rename dialog
	//
	preset_rename.set_title( "Rename Preset" );
	preset_rename.set_size_request( 300, 200 );
	preset_rename.set_resizable (false);
	preset_rename.get_vbox()->add( preset_rename_label );
	preset_rename_label.set_text( "Enter new Preset Name:" );
	preset_rename.get_vbox()->add( preset_rename_entry );
	preset_rename.get_action_area()->add( preset_rename_ok );
	preset_rename_ok.add_label( "Confirm", 0.5, 0.5 );
	preset_rename_ok.signal_clicked().connect(sigc::bind( sigc::mem_fun(*this, &GUI::event_handler), (int) evPresetRenameOk));
	preset_rename.get_action_area()->add( preset_rename_cancel );
	preset_rename_cancel.signal_clicked().connect(mem_fun(preset_rename, &Gtk::Dialog::hide));
	preset_rename_cancel.add_label( "Cancel", 0.5, 0.5 );
	preset_rename.set_modal( true );
	preset_rename.set_transient_for( *this );
	
	//
	// the new preset dialog
	//
	d_preset_new.set_title( "Create a New Preset" );
	d_preset_new.set_size_request( 300, 200 );
	d_preset_new.set_resizable (false);
	d_preset_new.get_vbox()->add( d_preset_new_label );
	d_preset_new_label.set_text( "Enter new Preset Name:" );
	d_preset_new.get_vbox()->add( d_preset_new_entry );
	d_preset_new.get_action_area()->add( d_preset_new_ok );
	d_preset_new_ok.add_label( "Confirm", 0.5, 0.5 );
	d_preset_new_ok.signal_clicked().connect(sigc::bind(mem_fun(*this, &GUI::event_handler), 0));
	d_preset_new.get_action_area()->add( d_preset_new_cancel );
	d_preset_new_cancel.signal_clicked().connect(mem_fun(d_preset_new, &Gtk::Dialog::hide));
	d_preset_new_cancel.add_label( "Cancel", 0.5, 0.5 );
	d_preset_new.set_modal( true );
	d_preset_new.set_transient_for( *this );
	
	
	aboutDlg.set_name (PACKAGE);
	aboutDlg.set_version (VERSION);
	aboutDlg.set_comments ("Analogue Modelling SYNTHesizer");
	aboutDlg.set_website ("http://code.google.com/p/amsynth/");
	std::string build_year(__DATE__, sizeof(__DATE__) - 5, 4);
	aboutDlg.set_copyright ("(C) 2002 - " + build_year + " Nick Dowell and others");
	Glib::RefPtr<Gdk::PixbufLoader> ldr = Gdk::PixbufLoader::create();
	ldr->write (amsynth_logo, sizeof(amsynth_logo)); ldr->close ();
	aboutDlg.set_logo (ldr->get_pixbuf());
	aboutDlg.signal_response().connect(sigc::hide(mem_fun(aboutDlg, &Gtk::Dialog::hide)));
	std::list<std::string> about_authors;
	about_authors.push_back("Nick Dowell");
	about_authors.push_back("Karsten Wiese");
	about_authors.push_back("Jezar at dreampoint");
	about_authors.push_back("Sebastien Cevey");
	about_authors.push_back("Taybin Rutkin");
	about_authors.push_back("Bob Ham"); 
	about_authors.push_back("Darrick Servis");
	about_authors.push_back("Johan Martinsson");
	about_authors.push_back("Andy Ryan");
	about_authors.push_back("Chris Cannam");
	about_authors.push_back("Paul Winkler");
	about_authors.push_back("Adam Sampson");
	about_authors.push_back("Martin Tarenskeen");
	about_authors.push_back("Adrian Knoth");
	about_authors.push_back("Samuli Suominen");
	aboutDlg.set_authors(about_authors);	
	
	//
	// the record dialog
	//
	record_dialog.set_title( "Capture Output" );
	record_dialog.add( record_vbox );
	record_dialog.set_resizable (false);
	
	record_vbox.set_spacing( 10 );
	record_vbox.pack_start( record_file_frame, TRUE, TRUE, 0 );
	record_vbox.pack_start( record_buttons_hbox, TRUE, TRUE, 0 );
	record_vbox.pack_start( record_statusbar, FALSE, FALSE, 0 );
	
	record_file_frame.set_border_width( 5 );
	record_file_frame.set_label( "output file:" );
	record_file_frame.add( record_file_hbox );
	record_file_hbox.set_border_width( 5 );
	record_file_hbox.set_spacing( 10 );
	record_file_hbox.add( record_entry );
	record_file_hbox.add( record_choose );
	record_entry.set_text( "amsynth-out.wav" );
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
	record_statusbar.push ("capture status: STOPPED", 1);

	gtk_window_set_icon_from_file(this->gobj(), DATADIR "/pixmaps/amsynth.png", NULL);
}

Gtk::MenuBar*
GUI::create_menus	( )
{
	using namespace Gtk::Menu_Helpers;
	using namespace Gtk;
	using sigc::bind;
	
	//
	// File menu
	//
	Menu *menu_file = manage (new Menu());
	MenuList& list_file = menu_file->items ();
	
#if defined(__linux)
	// create-new-instance currently only supported on Linux
	list_file.push_back (MenuElem("New Instance", sigc::bind(mem_fun(this, &GUI::event_handler),(int)evNewInstance)));
	list_file.push_back (SeparatorElem());
#endif
	
	list_file.push_back (MenuElem("_Open Bank",Gtk::AccelKey("<control>O"), mem_fun(*this, &GUI::bank_open)));
//	list_file.push_back (MenuElem("_Save Bank","<control>S", mem_fun(*this, &GUI::bank_save)));
	list_file.push_back (MenuElem("_Save Bank As...",Gtk::AccelKey("<control>S"), mem_fun(*this, &GUI::bank_save_as)));
	list_file.push_back (SeparatorElem());
	list_file.push_back (MenuElem("Open Alternate Tuning File", mem_fun(*this, &GUI::scale_open)));
	list_file.push_back (MenuElem("Open Alternate Keyboard Map", mem_fun(*this, &GUI::key_map_open)));
	list_file.push_back (MenuElem("Reset All Tuning Settings to Default", mem_fun(*this, &GUI::tuning_reset)));
	list_file.push_back (SeparatorElem());
	list_file.push_back (MenuElem("_Quit",Gtk::AccelKey("<control>Q"), bind(mem_fun(this, &GUI::event_handler),(int)evQuit)));
	
	
	//
	// Preset menu
	//
	Menu *menu_preset = manage (new Menu());
	MenuList& list_preset = menu_preset->items ();
//	list_preset.push_back (manage (new TearoffMenuItem ()));
	list_preset.push_back (MenuElem("_New", Gtk::AccelKey("<control>N"), mem_fun(*this, &GUI::preset_new)));
	list_preset.push_back (MenuElem("_Copy", Gtk::AccelKey("<control>C"), mem_fun(*this, &GUI::preset_copy)));
	list_preset.push_back (MenuElem("_Paste", Gtk::AccelKey("<control>V"), mem_fun(*this, &GUI::preset_paste)));
	list_preset.push_back (MenuElem("Paste as New", mem_fun(*this, &GUI::preset_paste_as_new)));
	list_preset.push_back (SeparatorElem());
	list_preset.push_back (MenuElem("Rename", sigc::bind(mem_fun(*this, &GUI::event_handler), (int)evPresetRename)));
	list_preset.push_back (MenuElem("Clear", bind(mem_fun(this,&GUI::event_handler),(int)evPresetDelete)));
	list_preset.push_back (SeparatorElem());
	list_preset.push_back (MenuElem("_Randomise", Gtk::AccelKey("<control>R"), sigc::mem_fun(preset_controller, &PresetController::randomiseCurrentPreset)));
	list_preset.push_back (MenuElem("Undo", Gtk::AccelKey("<control>Z"), sigc::mem_fun(preset_controller, &PresetController::undoChange)));
	list_preset.push_back (MenuElem("Redo", Gtk::AccelKey("<control>Y"), sigc::mem_fun(preset_controller, &PresetController::redoChange)));
	list_preset.push_back (SeparatorElem());
	list_preset.push_back (MenuElem("Import...", bind(mem_fun(*this, &GUI::event_handler), (int)evPresetImport)));
	list_preset.push_back (MenuElem("Export...", bind(mem_fun(*this, &GUI::event_handler), (int)evPresetExport)));

			
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
			ostringstream name; i ? name << i : name << "All";
			item = Gtk::manage(new Gtk::RadioMenuItem(grp, name.str()));
			item->set_active((i == currentValue));
			menu->items().push_back(*item);
		}
		
		// connect the signal handlers after calling set_active to prevent altering the value.
		for (int i=0; i<16; i++) {
			menu->items()[i].signal_activate().connect( sigc::bind(mem_fun(*this, &GUI::on_midi_channel_change), i) );
		}

		menu_config->items().push_back(MenuElem("MIDI Channel", *menu));
	}
	
	//
	// Config > Polyphony
	//
	{
		Gtk::RadioButtonGroup grp;
		Gtk::RadioMenuItem *item = NULL;
		Gtk::Menu *menu = Gtk::manage( new Gtk::Menu );
		const int currentValue = vau->GetMaxVoices();
		
		item = Gtk::manage(new Gtk::RadioMenuItem(grp, "Unlimited"));
		item->signal_activate().connect( sigc::bind(mem_fun(*this, &GUI::on_ployphony_change), 0, item) );
		menu->items().push_back(*item);
		
		for (int i=1; i<=16; i++) {
			ostringstream name; name << i;
			Gtk::RadioMenuItem *item = Gtk::manage(new Gtk::RadioMenuItem(grp, name.str()));
			item->set_active((i == currentValue));
			item->signal_activate().connect( sigc::bind(mem_fun(*this, &GUI::on_ployphony_change), i, item) );
			menu->items().push_back(*item);
		}
		
		menu_config->items().push_back(MenuElem("Max. Polyphony", *menu));
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
			name << " Semitones";
			Gtk::RadioMenuItem *item = Gtk::manage(new Gtk::RadioMenuItem(grp, name.str()));
			item->set_active(i == config->pitch_bend_range);
			item->signal_activate().connect( sigc::bind(mem_fun(*this, &GUI::on_pitch_bend_range_change), i, item) );
			m_pitchBendRangeMenu->items().push_back(*item);
		}

		m_pitchBendRangeMenu->signal_show().connect(sigc::mem_fun(*this, &GUI::on_pitch_bend_range_menu_show));

		menu_config->items().push_back(MenuElem("Pitch Bend Range", *m_pitchBendRangeMenu));
	}
	
	list_config.push_back (MenuElem("Audio & MIDI...", bind(mem_fun(*this, &GUI::event_handler), (int)evConfig)));
	
	
	//
	// Utils menu
	//
	Menu *menu_utils = manage (new Menu());
	MenuList& list_utils = menu_utils->items ();

	MenuItem *menu_item = manage (new MenuItem("Virtual Keyboard"));
	menu_item->signal_activate().connect(sigc::bind(mem_fun(*this, &GUI::event_handler),(int)evVkeybd));
	// vkeybd must exist, and we must be using ALSA MIDI
	if (config->alsa_seq_client_id == 0 || command_exists("vkeybd") != 0)
		menu_item->set_sensitive( false );
	list_utils.push_back (*menu_item);

	menu_item = manage (new MenuItem("Record to .wav file..."));
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
	if (config->alsa_seq_client_id==0) menu_item->set_sensitive( false );
	list_utils_midi.push_back (*menu_item);
	
	menu_item = manage (new MenuItem("alsa-patch-bay"));
	menu_item->signal_activate().connect (sigc::bind(mem_fun(*this, &GUI::command_run),"alsa-patch-bay --driver alsa"));
	if (command_exists ("alsa-patch-bay") != 0) menu_item->set_sensitive( false );
	if (config->alsa_seq_client_id==0) menu_item->set_sensitive( false );
	list_utils_midi.push_back (*menu_item);
	
	list_utils.push_back (MenuElem("MIDI (ALSA) connections", *menu_utils_midi));
	
	//
	// JACK sub-menu
	//
	Menu *menu_utils_jack = manage (new Menu());
	MenuList& list_utils_jack = menu_utils_jack->items ();
	
	menu_item = manage (new MenuItem("qjackconnect"));
	menu_item->signal_activate().connect (sigc::bind(mem_fun(*this, &GUI::command_run),"qjackconnect"));
	if (command_exists ("qjackconnect") != 0) menu_item->set_sensitive( false );
	if (config->current_audio_driver != "jack" && config->current_audio_driver != "JACK") menu_item->set_sensitive( false );
	list_utils_jack.push_back (*menu_item);
	
	menu_item = manage (new MenuItem("alsa-patch-bay"));
	menu_item->signal_activate().connect (sigc::bind(mem_fun(*this, &GUI::command_run),"alsa-patch-bay --driver jack"));
	if (command_exists ("alsa-patch-bay") != 0) menu_item->set_sensitive( false );
	if (config->current_audio_driver != "jack" && config->current_audio_driver != "JACK") menu_item->set_sensitive( false );
	list_utils_jack.push_back (*menu_item);
	
	list_utils.push_back (MenuElem("Audio (JACK) connections", *menu_utils_jack));
	
	list_utils.push_back (MenuElem("Send Settings to Midi", bind(mem_fun(this,&GUI::event_handler),(int)evMidiSend)));

	//
	// Help menu
	//
	Menu *menu_help = manage (new Menu());
	menu_help->items().push_back (MenuElem("About", mem_fun(aboutDlg, &Gtk::AboutDialog::show_all)));
	menu_help->items().push_back (MenuElem("Report a Bug...", bind(mem_fun(this, &GUI::event_handler), (int)evHelpMenuBugReport)));
	menu_help->items().push_back (MenuElem("Online Documentation...", bind(mem_fun(this, &GUI::event_handler), (int)evHelpMenuOnlineDocumentation)));

	
	//
	// Menubar
	//
	MenuBar *menu_bar = manage (new MenuBar ());
	
//	menu_bar->set_shadow_type( GTK_SHADOW_NONE );
	
	MenuList& list_bar = menu_bar->items();
	list_bar.push_back (MenuElem("_File", Gtk::AccelKey("<alt>F"), *menu_file));
	list_bar.push_back (MenuElem("_Preset", Gtk::AccelKey("<alt>P"), *menu_preset));
	list_bar.push_back (MenuElem("_Config", Gtk::AccelKey("<alt>C"), *menu_config));
	list_bar.push_back (MenuElem("_Utils", Gtk::AccelKey("<alt>U"), *menu_utils));
	list_bar.push_back (MenuElem("_Help", *menu_help));

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
	
	// Add custom signal for atomic change operations to parameters.
	g_signal_new(
		"start_atomic_value_change",
		g_type_from_name("GtkAdjustment"),
		G_SIGNAL_ACTION,
		0,
		NULL,
		NULL,
		NULL,
		G_TYPE_NONE,
		0
	);
	
	for (int i=0; i<kAmsynthParameterCount; i++) {
		Parameter &param = preset->getParameter(i);
		m_adjustments[i] = (GtkAdjustment *) gtk_adjustment_new (
			param.getValue(),
			param.getMin(),
			param.getMax(),
			param.getStep(),
			0, 0);
		gtk_signal_connect (GTK_OBJECT (m_adjustments[i]), "value_changed",
			(GtkSignalFunc) adjustment_value_changed,
			(gpointer) &param );

		m_undoArgs[i] = new UndoArgs(preset_controller, &param);
		g_signal_connect_after (G_OBJECT (m_adjustments[i]), "start_atomic_value_change",
			G_CALLBACK(start_atomic_adjustment_value_change),
			(gpointer) m_undoArgs[i] );
	}
	
	Gtk::Widget *editor = Glib::wrap (editor_pane_new (m_adjustments, FALSE));
	
	vbox.pack_start (*(create_menus ()),0,0);
	Gtk::HBox *tmphbox = manage (new Gtk::HBox());
	tmphbox->pack_start(*presetCV,0,0);

	vbox.pack_start (*tmphbox,0,0);
	vbox.pack_start (*editor, Gtk::PACK_EXPAND_WIDGET,0);
	vbox.pack_start (statusBar,PACK_SHRINK);
	add(vbox);
	
	
	// set up a fancy status bar.... why? i dont know.
	
	guint padding = 3;
	
	statusBar.pack_start (*manage(new Gtk::VSeparator), PACK_SHRINK);
	
	status = "MIDI : " + config->current_midi_driver;
	if (config->current_midi_driver == "OSS") status += " : " + config->oss_midi_device;
	statusBar.pack_start (*manage(new Gtk::Label (status)), PACK_SHRINK, padding);

	statusBar.pack_start (*manage(new Gtk::VSeparator), PACK_SHRINK);
	
	status = "Audio: " + config->current_audio_driver + " : ";
	if( config->current_audio_driver == "OSS" ) status += config->oss_audio_device;
	else if( config->current_audio_driver == "ALSA" ) status += config->alsa_audio_device;
	statusBar.pack_start (*manage(new Gtk::Label (status)), PACK_SHRINK, padding);
	
	statusBar.pack_start (*manage(new Gtk::VSeparator), PACK_SHRINK);
	
	ostringstream oss;
	oss << "Sample Rate: " << config->sample_rate;
	statusBar.pack_start (*manage(new Gtk::Label (oss.str())), PACK_SHRINK, padding);

	statusBar.pack_start (*manage(new Gtk::VSeparator), PACK_SHRINK);
	
#ifdef ENABLE_REALTIME
	if (config->current_audio_driver_wants_realtime)
	{
		status = "Realtime : ";
		status += config->realtime ? "YES" : "NO";
		statusBar.pack_start(*manage(new Gtk::Label (status)), PACK_SHRINK, padding);
	}
#endif

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
	bool bad_config = false;
	
	if (config->current_audio_driver.empty())
	{
		bad_config = true;
		MessageDialog dlg (*this, "amSynth configuration error", false, MESSAGE_ERROR, BUTTONS_OK, true);
		dlg.set_secondary_text(
			"amSynth could not initialise the selected audio device.\n\n"
			"Please review the configuration and restart"
		    );
		dlg.run();
	}
	
	if (config->current_midi_driver.empty())
	{
		bad_config = true;
		MessageDialog dlg (*this, "amSynth configuration error", false, MESSAGE_ERROR, BUTTONS_OK, true);
		dlg.set_secondary_text(
			"amSynth could not initialise the selected midi device.\n\n"
			"Please review the configuration and restart"
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
	if (config->current_audio_driver_wants_realtime == 1 &&
		config->realtime == 0)
	{
		MessageDialog dlg (*this, "amSynth could not set realtime priority");
		dlg.set_secondary_text ("You may experience audio buffer underruns resulting in 'clicks' in the audio.\n\nThis is most likely because the program is not SUID root.\n\nUsing the JACK audio subsystem can also help");
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
												   "Could not show link");
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", error->message);
		gtk_dialog_run(GTK_DIALOG(dialog));
		gtk_widget_destroy(dialog);
	}
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
	
	case evPresetDelete:
		{
			MessageDialog dlg (*this, "Delete the current Preset?", false, MESSAGE_QUESTION, BUTTONS_YES_NO, true);
			if (RESPONSE_YES == dlg.run())
			{
				preset_controller->deletePreset();
				preset_controller->commitPreset();
				preset_controller->selectPreset( 0 );
				presetCV->update();
			}
		}
		break;
	
	case evPresetExport:
		{
			FileChooserDialog dlg (*this, "Export preset as...", FILE_CHOOSER_ACTION_SAVE);
			dlg.add_button(Stock::CANCEL, RESPONSE_CANCEL);
			dlg.add_button(Stock::SAVE_AS, RESPONSE_OK);
			dlg.set_current_name (preset_controller->getCurrentPreset().getName()+".amSynthPreset");
			if (RESPONSE_OK == dlg.run()) preset_controller->exportPreset (dlg.get_filename());
		}
		break;
	
	case evPresetImport:
		{
			FileChooserDialog dlg (*this, "Import over current preset...", FILE_CHOOSER_ACTION_OPEN);
			dlg.add_button(Stock::CANCEL, RESPONSE_CANCEL);
			dlg.add_button("Select", RESPONSE_OK);
			FileFilter filter;
			filter.set_name("amSynth 1.x files");
			filter.add_pattern("*.amSynthPreset");
			dlg.add_filter(filter);
			if (RESPONSE_OK == dlg.run())
			{
				preset_controller->importPreset (dlg.get_filename());
			}
		}
		break;
	
	case evQuit:
		delete_event_impl(0);
		break;
	
	case evRecDlgFileChooser:
		{
			FileChooserDialog dlg (record_dialog, "Select output WAV file...", FILE_CHOOSER_ACTION_SAVE);
			dlg.add_button(Stock::CANCEL, RESPONSE_CANCEL);
			dlg.add_button(Stock::OK, RESPONSE_OK);
			dlg.set_current_name (record_entry.get_text());
			if (RESPONSE_OK == dlg.run()) record_entry.set_text (dlg.get_filename());
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
			record_statusbar.push ("capture status: RECORDING", 1);
		}
		break;
	
	case evRecDlgPause:
	    if( record_recording == true )
	    {
		    audio_out->stopRecording();
		    record_recording = false;
		    record_statusbar.pop( 1 );
		    record_statusbar.push ("capture status: STOPPED", 1);
	    }
	    break;
	
	case evVkeybd:
	    {
			char tmp[255] = "";
			snprintf(tmp, sizeof(tmp), "vkeybd --addr %d:0", config->alsa_seq_client_id);
			command_run(tmp);
	    }
		break;
	
	case evMidiSend:
		{
			MessageDialog dlg (*this, "Send Setting to Midi Out?", false, MESSAGE_QUESTION, BUTTONS_YES_NO, true);
			if (RESPONSE_YES == dlg.run()) midi_controller->sendMidi_values();
		}
		break;
		
	case evConfig:
	{
		ConfigDialog dlg (*this, *config);
		dlg.run ();
		break;
	}
	
	case evNewInstance:
		spawn_new_instance();
		break;

	case evHelpMenuBugReport:
		open_uri("http://code.google.com/p/amsynth/issues/list");
		break;

	case evHelpMenuOnlineDocumentation:
		open_uri("http://code.google.com/p/amsynth/w/list");
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
	std::ostringstream ostr;
	ostr << "amsynth";

	if (config->jack_client_name.length() && config->jack_client_name != "amsynth") {
		ostr << ": ";
		ostr << config->jack_client_name;
	}

	ostr << ": ";
	ostr << preset_controller->getCurrPresetNumber();

	ostr << ": ";
	ostr << preset_controller->getCurrentPreset().getName();

	if (m_presetIsNotSaved) {
		ostr << " *";
	}

#if ENABLE_MIDIKEYS
	if (m_vkeybdIsActive) {
		ostr << " (midikeys active)";
	}
#endif

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

void
GUI::preset_new		( )
{
	preset_controller->newPreset ();
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
	if (!pastedPreset.fromString(std::string(text)))
		return;
	
	// enure preset has a unique name
	for (int suffixNumber = 1; _this->preset_controller->containsPresetWithName(pastedPreset.getName()); suffixNumber++) {
		std::stringstream str; str <<  pastedPreset.getName() << " " << suffixNumber;
		pastedPreset.setName(str.str());
	}
	
	_this->preset_controller->getCurrentPreset() = pastedPreset;
	_this->presetCV->update();
}

void
GUI::preset_paste_as_new( )
{
	gtk_clipboard_request_text(gtk_clipboard_get(GDK_SELECTION_CLIPBOARD), GUI::preset_paste_as_new_callback, this);
}

void
GUI::preset_paste_as_new_callback(GtkClipboard *clipboard, const gchar *text, gpointer userdata)
{
	reinterpret_cast<GUI *>(userdata)->preset_new ();
	preset_paste_callback(clipboard, text, userdata);
}

////////////////////////////////////////////////////////////////////////////////

void
GUI::bank_open		( )
{
	FileChooserDialog dlg (*this, "Open amSynth Bank File...", FILE_CHOOSER_ACTION_OPEN);
	dlg.add_button(Stock::CANCEL, RESPONSE_CANCEL);	dlg.add_button("Select", RESPONSE_OK);
	if (RESPONSE_OK == dlg.run())
	{
		preset_controller->savePresets (config->current_bank_file.c_str ());
		config->current_bank_file = dlg.get_filename ();
		preset_controller->loadPresets (config->current_bank_file.c_str ());
	}
}

void
GUI::bank_save_as	( )
{
	GtkWidget *chooser = gtk_file_chooser_dialog_new (
		"Save preset bank",
		this->gobj(),
		GTK_FILE_CHOOSER_ACTION_SAVE,
		GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
		GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
		NULL);

	gtk_file_chooser_set_current_folder (GTK_FILE_CHOOSER (chooser), PresetController::getUserBanksDirectory().c_str());
	gtk_file_chooser_set_current_name (GTK_FILE_CHOOSER (chooser), "new.bank");
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
	FileChooserDialog dlg (*this, "Open Scala (.scl) alternate tuning file...", FILE_CHOOSER_ACTION_OPEN);
	dlg.add_button(Stock::CANCEL, RESPONSE_CANCEL);	dlg.add_button("Select", RESPONSE_OK);

	FileFilter filter;
	filter.set_name("Scala scale files");
	filter.add_pattern("*.[Ss][Cc][Ll]");
	dlg.add_filter(filter);

	if (dlg.run() == RESPONSE_OK)
	{
		dlg.hide();
		int error = vau->loadScale(dlg.get_filename());
		if (error)
		{
			MessageDialog msg(*this, "Failed to load new tuning.");
			msg.set_secondary_text("Reading the tuning file failed for some reason. \
Make sure your file has the correct format and try again.");
			msg.run();
		}
	}
}

void
GUI::key_map_open	( )
{
	FileChooserDialog dlg (*this, "Open alternate keybord map (Scala .kbm format)...", FILE_CHOOSER_ACTION_OPEN);
	dlg.add_button(Stock::CANCEL, RESPONSE_CANCEL);	dlg.add_button("Select", RESPONSE_OK);

	FileFilter filter;
	filter.set_name("Scala keyboard map files");
	filter.add_pattern("*.[Kk][Bb][Mm]");
	dlg.add_filter(filter);

	if (dlg.run() == RESPONSE_OK)
	{
		dlg.hide();
		int error = vau->loadKeyMap(dlg.get_filename());
		if (error)
		{
			MessageDialog msg(*this, "Failed to load new keyboard map.");
			msg.set_secondary_text("Reading the keyboard map file failed for some reason. \
Make sure your file has the correct format and try again.");
			msg.run();
		}
	}
}

void
GUI::tuning_reset	( )
{
	MessageDialog dlg (*this, "Discard the current scale and keyboard map?");

	if (dlg.run() == RESPONSE_OK)
	{
		vau->defaultTuning();
	}
}

// returns 0 if executable was found
int
GUI::command_exists	(const char *command)
{
	std::string cmdline = "which " + std::string(command);
	int result = system(cmdline.c_str());
	return result;
}

void
GUI::command_run	(const char *command)
{
	// TODO: would be better to fork() and exec()
	
	string full_command = std::string(command) + std::string(" &");
	// returns 0 even if command could not be run
	int result = system(full_command.c_str());
	result = 0;
}

void
GUI::on_midi_channel_change(int value)
{
	midi_controller->set_midi_channel(value);
}

void
GUI::on_ployphony_change(int value, Gtk::RadioMenuItem *item)
{
	if (item->get_active()) {
		if (config->polyphony != value) {
			config->polyphony = value;
			config->save();
		}
		vau->SetMaxVoices(value);
	}
}

void
GUI::on_pitch_bend_range_menu_show()
{
	Gtk::MenuShell::MenuList &list = m_pitchBendRangeMenu->items();
	guint index = MIN((vau->getPitchBendRangeSemitones() - 1), (list.size() - 1));
	Gtk::RadioMenuItem *item = (Gtk::RadioMenuItem *)&(list[index]);
	item->set_active();
}

void
GUI::on_pitch_bend_range_change(int value, Gtk::RadioMenuItem *item)
{
	if (item->get_active()) {
		if (config->pitch_bend_range != value) {
			config->pitch_bend_range = value;
			config->save();
			vau->setPitchBendRangeSemitones(config->pitch_bend_range);
		}
	}
}

#if ENABLE_MIDIKEYS

////////////////////////////////////////////////////////////////////////////////
//
// Virtual Keyboard functionality
//
// Uses the computer's typing keyboard to play the synth.
// Handy for testing and general playing with amSynth :)
//
//     W E   T Y U   O P
//    A S D F G H J K L ; '
//

#define SIZEOF_ARRAY( a ) ( sizeof(a) / sizeof((a)[0]) )

//
// List of hardware keycodes for midi notes
//
// Raw hardware keycodes are used because we want the physical layout
// to remain the same no matter what type of keyboard layout is used.
//
// Hardware keycodes seem to vary between platforms, although it's
// difficult to find any decent documentation that covers this :-/
//
// This particular layout is based on that used by Logic Studio.
//                                               
static guint16 s_vkeybd_hardware_keycodes[] = {
//	   A     W     S     E     D     F     T     G     Y     H     U     J     K     O     L     P     ;     '
#if defined(__linux)
	0x26, 0x19, 0x27, 0x1a, 0x28, 0x29, 0x1c, 0x2a, 0x1d, 0x2b, 0x1e, 0x2c, 0x2d, 0x20, 0x2e, 0x21, 0x2f, 0x30
#elif defined(__APPLE__)
	   8,   21,    9,   22,   10,   11,   25,   13,   24,   12,   40,   46,   48,   39,   45,   43,   49,   47
#endif
};

//
// returns -1 if hardware_keycode is not mapped to a midi note
//
static inline char midi_note_for_hardware_keycode( guint16 hardware_keycode )
{
	const guint16 kGreatestValidKeycode = 64;
	static char value_for_code[kGreatestValidKeycode+1] = {0};
	static bool initialised = false;
	
	// first-time initialisation (build the lookup table)
	if (initialised == false) { initialised = true;
		for (unsigned note=0; note<SIZEOF_ARRAY(s_vkeybd_hardware_keycodes); note++) {
			guint16 code = s_vkeybd_hardware_keycodes[note];
			value_for_code[code] = note + 1;
		}
	}
	
	if (hardware_keycode < kGreatestValidKeycode)
		return value_for_code[hardware_keycode] - 1;

	return -1;
}

bool GUI::on_key_press_event(GdkEventKey *inEvent)
{
	char midiNote = -1;

//	fprintf(stderr, "string '%s' keyval 0x%02x keycode 0x%02x\n",
//			inEvent->string,
//			inEvent->keyval,
//			inEvent->hardware_keycode);
	
	if (inEvent->keyval == GDK_Caps_Lock) {
		if ((inEvent->state & GDK_LOCK_MASK) == GDK_LOCK_MASK) {
			// will be disabled by this key press
			m_vkeybdIsActive = false;
			vkeybd_kill_all_notes();
		} else {
			m_vkeybdIsActive = true;
		}
		update_title();
	}
	
	if ((inEvent->state & GDK_LOCK_MASK) == 0)
		goto delegate;
	
	//
	// switch between octaves using the number keys
	//
	switch (inEvent->keyval) {
		case GDK_1: m_vkeybdOctave = 0; vkeybd_kill_all_notes(); return true;
		case GDK_2: m_vkeybdOctave = 1; vkeybd_kill_all_notes(); return true;
		case GDK_3: m_vkeybdOctave = 2; vkeybd_kill_all_notes(); return true;
		case GDK_4: m_vkeybdOctave = 3; vkeybd_kill_all_notes(); return true;
		case GDK_5: m_vkeybdOctave = 4; vkeybd_kill_all_notes(); return true;
		case GDK_6: m_vkeybdOctave = 5; vkeybd_kill_all_notes(); return true;
		case GDK_7: m_vkeybdOctave = 6; vkeybd_kill_all_notes(); return true;
		case GDK_8: m_vkeybdOctave = 7; vkeybd_kill_all_notes(); return true;
		case GDK_9: m_vkeybdOctave = 8; vkeybd_kill_all_notes(); return true;
		case GDK_0: m_vkeybdOctave = 9; vkeybd_kill_all_notes(); return true;
	}
	
	if ((midiNote = midi_note_for_hardware_keycode( inEvent->hardware_keycode )) == -1)
		goto delegate;
	
	midiNote += (m_vkeybdOctave * 12);
	if (m_vkeybdState[midiNote] == false) {
		m_vkeybdState[midiNote]  = true;
		vau->HandleMidiNoteOn(midiNote, 0.7f);
	}
	return true;
	
delegate:
	return Gtk::Window::on_key_press_event(inEvent);
}

bool GUI::on_key_release_event(GdkEventKey *inEvent)
{
	char midiNote = -1;
	
	if ((inEvent->state & GDK_LOCK_MASK) != GDK_LOCK_MASK)
		goto delegate;
	
	if ((midiNote = midi_note_for_hardware_keycode( inEvent->hardware_keycode )) == -1)
		goto delegate;
	
#if defined(__linux)
	// XkbSetDetectableAutoRepeat() seems to be broken on some Linux configurations,
	// which means we can receive key release events for autorepeat. Therefore, we
	// need to filter out these 'fake' key release events.
	// http://bugs.freedesktop.org/show_bug.cgi?id=22515
	{
		char pressed_keys[32];
		XQueryKeymap(gdk_x11_get_default_xdisplay(), pressed_keys);
		bool isPressed = (pressed_keys[inEvent->hardware_keycode >> 3] >> (inEvent->hardware_keycode & 0x07)) & 0x01;
		if (isPressed) {
			goto delegate;
		}
	}
#endif

	midiNote += (m_vkeybdOctave * 12);
	if (m_vkeybdState[midiNote] == true) {
		m_vkeybdState[midiNote]  = false;
		vau->HandleMidiNoteOff(midiNote, 0.7f);
	}
	return true;
	
delegate:
	return Gtk::Window::on_key_release_event(inEvent);
}

void GUI::vkeybd_kill_all_notes()
{
	for (unsigned i=0; i<m_vkeybdState.size(); i++) {
		vau->HandleMidiNoteOff(i, 0.0f);
		m_vkeybdState[i] = false;
	}
}

#else

bool GUI::on_key_press_event(GdkEventKey *event)
{
	if (event->keyval == GDK_a && event->state & GDK_CONTROL_MASK) {
		if (!m_auditionKeyDown) {
			m_auditionKeyDown = true;
			int note = presetCV->getAuditionNote();
			vau->HandleMidiNoteOn(note, 1.0f);
		}
		return true;
	}
	return Gtk::Window::on_key_press_event(event);
}

bool GUI::on_key_release_event(GdkEventKey *event)
{
	if (event->keyval == GDK_a && event->state & GDK_CONTROL_MASK) {
		if (m_auditionKeyDown) {
			m_auditionKeyDown = false;
			int note = presetCV->getAuditionNote();
			vau->HandleMidiNoteOff(note, 1.0f);
		}
		return true;
	}
	return Gtk::Window::on_key_release_event(event);
}

#endif
