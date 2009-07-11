/* amSynth
 * (c) 2002-2006 Nick Dowell
 * portions of this file (c) 2003 Darrick Servis
 */
 
#include "GUI.h"

#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <sys/types.h>

#include <gtkmm.h>
#include <sigc++/bind.h>

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
	MessageDialog dlg (*this, "Really quit amSynth?\n\nYou will lose any changes\nwhich you haven't explicitly commited", false, MESSAGE_QUESTION, BUTTONS_YES_NO, true);
	if (RESPONSE_YES == dlg.run()) hide_all();
	return true;
}

GUI::GUI( Config & config, MidiController & mc, VoiceAllocationUnit & vau,
		int pipe_write, GenericOutput *audio, const char *title )
:	controller_map_dialog(NULL)
,	clipboard_preset (new Preset)
{
	lnav = -1;
	
	this->config = &config;
	this->midi_controller = &mc;
	this->m_pipe_write = pipe_write;
	this->vau = &vau;
	this->audio_out = audio;
	
//	if(this->config->realtime)
		// messes up the audio thread's timing if not realtime...
//		Gtk::Main::timeout.connect( mem_fun(*this,&GUI::idle_callback), 200 );

        m_baseName = title + string(" : ");
	set_title(m_baseName);
	set_resizable(false);
        
        m_requestUpdate.slot = mem_fun(*this, &GUI::onUpdate);

	active_param = 0;


//	style = Gtk::Style::create ( );
	
	
	presetCV = new PresetControllerView(pipe_write, *this->vau);

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
	std::string build_year(__DATE__, sizeof(__DATE__) - 5);
	aboutDlg.set_copyright ("(C) 2002 - " + build_year + " Nick Dowell and others");
	Glib::RefPtr<Gdk::PixbufLoader> ldr = Gdk::PixbufLoader::create();
	ldr->write (amsynth_logo, sizeof(amsynth_logo)); ldr->close ();
	aboutDlg.set_logo (ldr->get_pixbuf());
	aboutDlg.signal_response().connect(sigc::hide(mem_fun(aboutDlg, &Gtk::Dialog::hide)));
	
	
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
	if (config->alsa_seq_client_id==0) menu_item->set_sensitive( false );
	// test for presence of vkeybd.
	if (command_exists ("vkeybd") != 0) menu_item->set_sensitive( false );
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
	
	editor_panel = new EditorPanel (preset, m_pipe_write);
	
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
	
	status = "Realtime : ";
	if (this->config->current_audio_driver!="jack" && 
		this->config->current_audio_driver!="JACK" &&
		config->realtime
		){ status += "YES"; }
	else { status += "NO"; }
	statusBar.pack_start (*manage(new Gtk::Label (status)), PACK_SHRINK, padding);
	
	show_all();
	
	
	// show realtime warning message if necessary
	if (!config->realtime && config->current_audio_driver != "jack" && config->current_audio_driver != "JACK")
	{
		MessageDialog dlg (*this, "amSynth could not set realtime priority");
		dlg.set_secondary_text ("You may experience audio buffer underruns resulting in 'clicks' in the audio.\n\nThis is most likely because the program is not SUID root.\n\nUsing the JACK audio subsystem can also help");
		dlg.run();
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
		    string tmp = "vkeybd --addr ";
		    char tc[5];
		    sprintf( tc, "%d", config->alsa_seq_client_id );
		    tmp += string(tc);
		    tmp += ":0 &";
		    system( tmp.c_str() );
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
    write(m_pipe_write, &m_requestUpdate, sizeof(Request));
}
void
GUI::onUpdate()
{
    set_title(m_baseName + preset_controller->getCurrentPreset().getName());
    presetCV->update();
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
	
    controller_map_dialog = new ControllerMapDialog(m_pipe_write, midi_controller, preset_controller);
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

int
GUI::command_exists	(const char *command)
{
	string test_command = "which ";
	test_command += command;
	test_command += " &> /dev/null";
	
	int res = system (test_command.c_str ());
	
	if (WEXITSTATUS (res) == 0)
		// exit code 0 - program exists in $PATH
		return 0;
	else
		return 1;
}

void
GUI::command_run	(const char *command)
{
	string full_command = command;
	full_command += " &";
	
	system (full_command.c_str ());
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

void GUI::GdkInputFunction(gpointer data, gint source, GdkInputCondition condition)
{
	static Request req;
	if (read (source, &req, sizeof(Request)) == sizeof(Request)) req.slot();

}

void ShowModalErrorMessage(const string & msg)
{
	MessageDialog dlg ("amSynth", false, MESSAGE_ERROR, BUTTONS_OK, true);
	dlg.set_secondary_text(msg);
	dlg.run();
}

