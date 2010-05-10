/* amSynth
 * (c) 2002-2006 Nick Dowell
 * portions of this file (c) 2003 Darrick Servis
 */
 
#include "GUI.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <sys/types.h>

#include <gtkmm.h>
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

#include "EditorPanel.h"
#include "../../config.h"
#include "amsynth_logo.h"
#include "ConfigDialog.h"

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
};

int
GUI::delete_event_impl(GdkEventAny *)
{
	if (m_presetIsNotSaved) {
		MessageDialog dlg (*this, "Really quit amSynth?\n\nYou will lose any changes\nwhich you haven't explicitly commited", false, MESSAGE_QUESTION, BUTTONS_YES_NO, true);
		if (RESPONSE_YES != dlg.run())
			return false;
	}
	hide_all();
	return true;
}

GUI::GUI( Config & config_in, MidiController & mc, VoiceAllocationUnit & vau_in,
		  GenericOutput *audio, const char *title )
:	controller_map_dialog(NULL)
,	clipboard_preset (new Preset)
,	m_vkeybdOctave(4)
,	m_vkeybdIsActive(false)
,	m_vkeybdState(128)
{
	lnav = -1;
	
	this->config = &config_in;
	this->midi_controller = &mc;
	this->vau = &vau_in;
	this->audio_out = audio;
	
//	if(this->config->realtime)
		// messes up the audio thread's timing if not realtime...
//		Gtk::Main::timeout.connect( mem_fun(*this,&GUI::idle_callback), 200 );

        m_baseName = title + string(" : ");
	set_title(m_baseName);
	set_resizable(false);
        
	active_param = 0;
	
//	style = Gtk::Style::create ( );
	
	
	presetCV = new PresetControllerView(*this->vau);

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
	aboutDlg.set_website ("http://amsynthe.sourceforge.net/amSynth");
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
	std::list<std::string> about_artists;
	about_artists.push_back("Saul Cross");
	aboutDlg.set_artists(about_artists);
	
	
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
	list_file.push_back (MenuElem("New Instance", sigc::bind(mem_fun(*this, &GUI::command_run),"amSynth")));
	list_file.push_back (SeparatorElem());
	list_file.push_back (MenuElem("_Open Bank",Gtk::AccelKey("<control>O"), mem_fun(*this, &GUI::bank_open)));
//	list_file.push_back (MenuElem("_Save Bank","<control>S", mem_fun(*this, &GUI::bank_save)));
	list_file.push_back (MenuElem("_Save Bank As...",Gtk::AccelKey("<control>S"), mem_fun(*this, &GUI::bank_save_as)));
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
	list_preset.push_back (MenuElem("_Randomise", Gtk::AccelKey("<control>R"), sigc::mem_fun(preset_controller->getCurrentPreset(), &Preset::randomise)));
	list_preset.push_back (SeparatorElem());
	list_preset.push_back (MenuElem("Import...", bind(mem_fun(*this, &GUI::event_handler), (int)evPresetImport)));
	list_preset.push_back (MenuElem("Export...", bind(mem_fun(*this, &GUI::event_handler), (int)evPresetExport)));

			
	//
	// Config menu
	//
	Menu *menu_config = manage (new Menu());
	MenuList& list_config = menu_config->items ();
	list_config.push_back (MenuElem("Audio & MIDI...", bind(mem_fun(*this, &GUI::event_handler), (int)evConfig)));
	list_config.push_back (MenuElem("MIDI Controllers...", Gtk::AccelKey("<control>M"), mem_fun(*this, &GUI::config_controllers)));
	
	
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
	// Menubar
	//
	MenuBar *menu_bar = manage (new MenuBar ());
	
//	menu_bar->set_shadow_type( GTK_SHADOW_NONE );
	
	MenuList& list_bar = menu_bar->items();
	list_bar.push_back (MenuElem("_File", Gtk::AccelKey("<alt>F"), *menu_file));
	list_bar.push_back (MenuElem("_Preset", Gtk::AccelKey("<alt>P"), *menu_preset));
	list_bar.push_back (MenuElem("_Config", Gtk::AccelKey("<alt>C"), *menu_config));
	list_bar.push_back (MenuElem("_Utils", Gtk::AccelKey("<alt>U"), *menu_utils));
	
	
	menu_item = manage (new MenuItem("Analogue Modelling SYNTHesizer"));
	menu_item->set_right_justified (true);
	menu_item->signal_activate().connect(mem_fun(aboutDlg, &Gtk::AboutDialog::show_all));
	list_bar.push_back (*menu_item);
	
	return menu_bar;
}


void
GUI::config_controllers()
{
	if (controller_map_dialog) controller_map_dialog->show_all();
}



void 
GUI::init()
{
	Preset *preset = &(preset_controller->getCurrentPreset());
	
	editor_panel = new EditorPanel (preset);
	
	adj_midi = manage (new Gtk::Adjustment(config->midi_channel,0,16,1));
	adj_midi->signal_value_changed().connect (mem_fun (*this, &GUI::changed_midi_channel));
	Gtk::SpinButton *sb_midi = manage (new Gtk::SpinButton(*adj_midi,1,0));
	
	adj_voices = manage (new Gtk::Adjustment(config->polyphony,1,128,1));
	adj_voices->signal_value_changed().connect (mem_fun (*this, &GUI::changed_voices));
	Gtk::SpinButton *sb_voices = manage (new Gtk::SpinButton(*adj_voices,1,0));
	
	vbox.pack_start (*(create_menus ()),0,0);
	Gtk::HBox *tmphbox = manage (new Gtk::HBox());
	tmphbox->pack_start(*presetCV,0,0);
	Gtk::HBox *midibox = manage(new Gtk::HBox());
	midibox->pack_start(*(manage( new Gtk::Label (" midi ch:") )),0,0);
	midibox->pack_start(*sb_midi,0,0);
	midibox->pack_start(*(manage( new Gtk::Label (" poly:") )),0,0);
	midibox->pack_start(*sb_voices,0,0);
	Gtk::Alignment *align = manage(new Gtk::Alignment(Gtk::ALIGN_RIGHT,
							  Gtk::ALIGN_CENTER,
							  0,0));
	align->add(*midibox);
	tmphbox->pack_start(*align);
	vbox.pack_start (*tmphbox,0,0);
	vbox.pack_start (*editor_panel,Gtk::PACK_EXPAND_WIDGET,0);
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
	
	static char cstr[32];
	sprintf( cstr, "Sample Rate: %d", config->sample_rate );
	statusBar.pack_start (*manage(new Gtk::Label (cstr)), PACK_SHRINK, padding);

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
		
	default:
		cout << "no handler for event: " << e << endl;
		break;
    }
}

gint
GUI::idle_callback()
{
	if (vau->GetActiveVoices() != lnav)
	{
		string txt = status;
		txt += "   ";
		char cstr[3];
		sprintf (cstr, "%d", vau->GetActiveVoices());
		txt += string( cstr );
		if( config->polyphony != 0 )
		{
			sprintf( cstr, "%d", config->polyphony );
			txt += "/";
			txt += string(cstr);
		}
		txt += " Voices Active";
		statusBar.pop( 1 );
		statusBar.push (txt, 1);
	}
	return true;
}

GUI::~GUI()
{
	delete controller_map_dialog;
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
	std::string title = m_baseName + preset_controller->getCurrentPreset().getName();
	if (m_presetIsNotSaved) {
		title += std::string(" *");
	}
	if (m_vkeybdIsActive) {
		title += std::string(" (midikeys active)");
	}
	set_title(title);
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
	if (0 <= paramID && paramID < kControls_End)
	{
		char tmpstr[32] = "";
		snprintf (tmpstr, 32, "%s = %.3f",
			preset_controller->getCurrentPreset().getParameter(paramID).getName().c_str(),
			preset_controller->getCurrentPreset().getParameter(paramID).getControlValue());
		statusBar.pop (0);
		statusBar.push (Glib::ustring (tmpstr), 0);
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
    presetCV->setPresetController(*preset_controller);
	onUpdate();
	
	// register for notification of all parameter changes
	Preset &preset = preset_controller->getCurrentPreset();
	unsigned paramCount = preset.ParameterCount();
	for (unsigned i=0; i<paramCount; i++) {
		preset.getParameter(i).addUpdateListener(*this);
	}
	
    controller_map_dialog = new ControllerMapDialog(midi_controller, preset_controller);
}

void
GUI::preset_new		( )
{
	preset_controller->newPreset ();
}

void
GUI::preset_copy	( )
{
	*clipboard_preset = preset_controller->getCurrentPreset ();
}

void
GUI::preset_paste	( )
{
	preset_controller->getCurrentPreset() = *clipboard_preset;
}

void
GUI::preset_paste_as_new( )
{
	preset_new ();
	preset_controller->getCurrentPreset() = *clipboard_preset;
}

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
	/* we need to get the filename entry widget back in there somehow.... */
	FileChooserDialog dlg (*this, "Save As...", FILE_CHOOSER_ACTION_SAVE);
	dlg.add_button(Stock::CANCEL, RESPONSE_CANCEL);
	dlg.add_button(Stock::SAVE_AS, RESPONSE_OK);
	dlg.set_current_name ("default.amsynth.bank");
	if (RESPONSE_OK == dlg.run())
	{
		config->current_bank_file = dlg.get_filename ();
		preset_controller->savePresets (config->current_bank_file.c_str ());
	}
}

static gchar *which(gchar *command)
{
	gint exit_status = -1;
	gchar *standard_output = NULL;
	gchar *argv[] = { (gchar *)"/usr/bin/which", (gchar *)command };
	g_spawn_sync(NULL, argv, NULL, G_SPAWN_STDERR_TO_DEV_NULL, NULL, NULL, &standard_output, NULL, &exit_status, NULL);
	if (exit_status != 0)
	{
		g_free(standard_output);
		return NULL;
	}
	return standard_output;
}

// returns 0 if executable was found
int
GUI::command_exists	(const char *command)
{
	gchar *path = which((gchar *)command);
	if (path) g_free(path);
	return path == NULL ? 1 : 0;
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
GUI::changed_midi_channel( )
{
	if (midi_controller) midi_controller->set_midi_channel ((int)(adj_midi->get_value ()));
}

void
GUI::changed_voices	( )
{
	config->polyphony = (int)(adj_voices->get_value ());
	if (vau) vau->SetMaxVoices (config->polyphony);
}

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
	return Gtk::Window::on_key_press_event(inEvent);
}

void GUI::vkeybd_kill_all_notes()
{
	for (unsigned i=0; i<m_vkeybdState.size(); i++) {
		vau->HandleMidiNoteOff(i, 0.0f);
		m_vkeybdState[i] = false;
	}
}
