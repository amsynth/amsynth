/* amSynth
 * (c) 2002-2003 Nick Dowell
 * portions of this file (c) 2003 Darrick Servis
 */
 
#include "GUI.h"

#include <stdio.h>
#include <stdlib.h>
#include <list>
#include <sys/types.h>
#include <sys/wait.h>

#include "EditorPanel.h"

#include "splash.xpm"


using SigC::slot;
using SigC::bind;

int
GUI::delete_event_impl(GdkEventAny *)
{
    event_handler( "quit" );
    return true;
}

void
GUI::serve_request()
{
	Request *request;
	request = (Request*)malloc( sizeof(Request) );
	  
	if( read( pipe[0], request, sizeof(Request) ) != sizeof(Request) )
		cout << "error reading from pipe" << endl;
	else {
		request->slot.call();
		free(request);
	}
}

void
GUI::set_x_font 	( const char *x_font_name )
{
	xfontname = x_font_name;
	font_sel.set_font_name (x_font_name);
	editor_panel->set_x_font (x_font_name);
}

GUI::GUI( Config & config, MidiController & mc, VoiceAllocationUnit & vau,
		int pipe[2], GenericOutput *audio, const char *title )
{
#ifdef _DEBUG
	cout << "<GUI::GUI()>" << endl;
#endif
	lnav = -1;
	
	this->config = &config;
	this->midi_controller = &mc;
	this->pipe = pipe;
	this->vau = &vau;
	this->audio_out = audio;
	hide.connect( Gtk::Main::quit.slot() );
	
	if(this->config->realtime)
		// messes up the audio thread's timing if not realtime...
		Gtk::Main::timeout.connect( slot(this,&GUI::idle_callback), 200 );

	set_title( title );
	set_policy( false, false, false );

	active_param = 0;


	style = Gtk::Style::create ( );
	
	
	presetCV = new PresetControllerView( pipe[1], *this->vau );

#ifdef _DEBUG
	cout << "<GUI::GUI()> created presetCV" << endl;
#endif

		
#ifdef _DEBUG
	cout << "<GUI::GUI()> put all controls" << endl;
#endif
	show_all();
	
	//
	// the preset rename dialog
	//
	preset_rename.set_title( "Rename Preset" );
	preset_rename.set_usize( 300, 200 );
	preset_rename.get_vbox()->add( preset_rename_label );
	preset_rename_label.set_text( "Enter new Preset Name:" );
	preset_rename.get_vbox()->add( preset_rename_entry );
	preset_rename.get_action_area()->add( preset_rename_ok );
	preset_rename_ok.add_label( "Confirm", 0.5, 0.5 );
	preset_rename_ok.clicked.connect(
		bind(slot(this, &GUI::event_handler),"preset::rename::ok"));
	preset_rename.get_action_area()->add( preset_rename_cancel );
	preset_rename_cancel.clicked.connect( preset_rename.hide.slot() );
	preset_rename_cancel.add_label( "Cancel", 0.5, 0.5 );
	preset_rename.set_modal( true );
	preset_rename.set_transient_for( *this );
	preset_rename.delete_event.connect( 
		bind( slot( this, &GUI::delete_events ), &preset_rename ) );
	
	//
	// the new preset dialog
	//
	d_preset_new.set_title( "Create a New Preset" );
	d_preset_new.set_usize( 300, 200 );
	d_preset_new.get_vbox()->add( d_preset_new_label );
	d_preset_new_label.set_text( "Enter new Preset Name:" );
	d_preset_new.get_vbox()->add( d_preset_new_entry );
	d_preset_new.get_action_area()->add( d_preset_new_ok );
	d_preset_new_ok.add_label( "Confirm", 0.5, 0.5 );
	d_preset_new_ok.clicked.connect(
		bind(slot(this, &GUI::event_handler),"preset::new::ok"));
	d_preset_new.get_action_area()->add( d_preset_new_cancel );
	d_preset_new_cancel.clicked.connect( d_preset_new.hide.slot() );
	d_preset_new_cancel.add_label( "Cancel", 0.5, 0.5 );
	d_preset_new.set_modal( true );
	d_preset_new.set_transient_for( *this );
	d_preset_new.delete_event.connect( 
		bind( slot( this, &GUI::delete_events ), &d_preset_new ) );
	
	//
	// the delete preset dialog
	//
	preset_delete.set_title( "Delete Preset?" );
	preset_delete.set_usize( 300, 200 );
	preset_delete.get_vbox()->add( preset_delete_label );
	preset_delete_label.set_text( "Delete the current Preset?" );
	preset_delete.get_action_area()->add( preset_delete_ok );
	preset_delete_ok.add_label( "Yes", 0.5, 0.5 );
	preset_delete_ok.clicked.connect(
		bind(slot(this, &GUI::event_handler),"preset::delete::ok"));
	preset_delete.get_action_area()->add( preset_delete_cancel );
	preset_delete_cancel.add_label( "Cancel", 0.5, 0.5 );
	preset_delete_cancel.clicked.connect( preset_delete.hide.slot() );
	preset_delete.set_modal( true );
	preset_delete.set_transient_for( *this );
	preset_delete.delete_event.connect( 
		bind( slot( this, &GUI::delete_events ), &preset_delete ) );
	
	//
	// the about window
	//
	about_window.set_title( "About" );
	about_window.get_vbox()->add( *about_pixmap );
	about_window.get_action_area()->add( about_close_button );
	about_close_button.add_label( "sweet", 0.5, 0.5 );
	about_close_button.clicked.connect( about_window.hide.slot() );
	about_window.set_transient_for( *this );
	about_window.delete_event.connect( 
		bind( slot( this, &GUI::delete_events ), &about_window ) );

	//
	// Bank Open dialog
	//
	d_bank_open.set_title( "Open amSynth Bank File..." );
	d_bank_open.get_cancel_button()->clicked.connect(
		d_bank_open.hide.slot() );
	d_bank_open.get_ok_button()->clicked.connect(
		slot(this, &GUI::bank_open_ok));
	d_bank_open.set_modal( true );
	d_bank_open.set_transient_for( *this );
	d_bank_open.delete_event.connect( 
		bind( slot( this, &GUI::delete_events ), &d_bank_open ) );
	
	//
	// Bank SaveAs dialog
	//
	d_bank_save_as.set_title( "Save As..." );
	d_bank_save_as.get_cancel_button()->clicked.connect(
		d_bank_save_as.hide.slot() );
	d_bank_save_as.get_ok_button()->clicked.connect(
		slot(this, &GUI::bank_save_as_ok));
	d_bank_save_as.set_modal( true );
	d_bank_save_as.set_transient_for( *this );
	d_bank_save_as.delete_event.connect( 
		bind( slot( this, &GUI::delete_events ), &d_bank_save_as ) );

		
	//
	// export dialog
	//
	preset_export_dialog.set_title( "Select DIRECTORY to export preset to" );
	preset_export_dialog.get_cancel_button()->clicked.connect(
		preset_export_dialog.hide.slot() );
	preset_export_dialog.get_ok_button()->clicked.connect(
		bind(slot(this, &GUI::event_handler),"preset::export::ok"));
	preset_export_dialog.get_selection_entry()->set_editable( false );
	preset_export_dialog.set_modal( true );
	preset_export_dialog.set_transient_for( *this );
	preset_export_dialog.delete_event.connect( 
		bind( slot( this, &GUI::delete_events ), &preset_export_dialog ) );
	
	//
	// import dialog
	//
	preset_import_dialog.set_title( "Import as current preset" );
	preset_import_dialog.get_cancel_button()->clicked.connect(
		preset_import_dialog.hide.slot() );
	preset_import_dialog.get_ok_button()->clicked.connect(
		bind(slot(this, &GUI::event_handler),"preset::import::ok"));
	preset_import_dialog.set_modal( true );
	preset_import_dialog.set_transient_for( *this );
	preset_import_dialog.delete_event.connect( 
		bind( slot( this, &GUI::delete_events ), &preset_import_dialog ) );
	
	//
	// record file-selector dialog
	//
	record_fileselect.set_title( "set output WAV file" );
	record_fileselect.set_filename( "/tmp/amSynth-out.wav" );
	record_fileselect.get_cancel_button()->clicked.connect(
		record_fileselect.hide.slot() );
	record_fileselect.get_ok_button()->clicked.connect(
		bind( slot(this, &GUI::event_handler), "record::fileselect::ok" ));
	record_fileselect.set_modal( true );
	record_fileselect.set_transient_for( *this );
	record_fileselect.delete_event.connect(
		bind( slot(this, &GUI::delete_events), &record_fileselect ));
	
	//
	// the record dialog
	//
	record_dialog.set_title( "Capture Output" );
	preset_import_dialog.set_transient_for( *this );
	record_dialog.add( record_vbox );
	record_dialog.delete_event.connect( bind( slot(this, &GUI::delete_events), &record_dialog ) );
	
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
	record_entry.set_text( "/tmp/amSynth-out.wav" );
	record_choose.add_label( "...", 0.5, 0.5 );
	record_choose.clicked.connect( bind(slot(this, &GUI::event_handler),"record_dialog::choose"));
		
	record_buttons_hbox.add( record_record );
	record_buttons_hbox.add( record_pause );
	record_buttons_hbox.set_border_width( 10 );
	record_buttons_hbox.set_spacing( 10 );
	record_record.add_label( "REC", 0.5, 0.5 );
	record_pause.add_label( "STOP", 0.5, 0.5 );
	record_record.clicked.connect( bind(slot(this, &GUI::event_handler),"record_dialog::record") );
	record_pause.clicked.connect( bind(slot(this, &GUI::event_handler),"record_dialog::pause") );
	
	record_recording = false;
	record_statusbar.push( 1, "capture status: STOPPED" );
	
	//
	// the quit confirmation window
	//
	quit_confirm.set_title( "Quit?" );
	quit_confirm.set_usize( 300, 200 );
	quit_confirm.get_vbox()->add( quit_confirm_label );
	quit_confirm_label.set_text( "Really quit amSynth?\n\nYou will lose any changes\nwhich you haven't explicitly commited" );
	quit_confirm.get_action_area()->add( quit_confirm_ok );
	quit_confirm_ok.add_label( "Yes, Quit!", 0.5, 0.5 );
	quit_confirm_ok.clicked.connect(
		bind(slot(this, &GUI::event_handler),"quit::ok") );
	quit_confirm.get_action_area()->add( quit_confirm_cancel );
	quit_confirm_cancel.add_label( "Cancel", 0.5, 0.5 );
	quit_confirm_cancel.clicked.connect( quit_confirm.hide.slot() );
	quit_confirm.set_modal( true );
	quit_confirm.set_transient_for( *this );
	quit_confirm.delete_event.connect( bind( slot( this, &GUI::delete_events ), &quit_confirm ) );

	//
	// font selection dialog
	//
	font_sel.get_ok_button()->clicked.connect(
			bind(slot(this, &GUI::event_handler),"font::ok") );
	font_sel.get_apply_button()->clicked.connect(
			bind(slot(this, &GUI::event_handler),"font::apply") );
	font_sel.get_cancel_button()->clicked.connect(
			bind(slot(this, &GUI::event_handler),"font::cancel") );
	font_sel.delete_event.connect( 
			bind(slot(this, &GUI::delete_events), &font_sel ) );

	//
	// show realtime warning message if necessary
	//
	if(!(this->config->realtime))
		if (!(this->config->realtime))
		// dont care if using JACK..
		if (!( this->config->audio_driver=="jack" ||
				this->config->audio_driver=="JACK" ))
	{
		realtime_warning.set_title("Warning");
		realtime_warning.set_usize( 400, 200 );
		realtime_text_label.set_text(
		" amSynth could not set realtime priority. \n You may experience audio buffer underruns \n resulting in 'clicks' in the audio.\n This is most likely because the program is not SUID root.\n Please read the documentation for information on how to remedy this.");
		realtime_warning.get_vbox()->add( realtime_text_label );
		realtime_warning.get_action_area()->add( realtime_close_button );
		realtime_close_button.add_label("close", 0.5, 0.5);
		realtime_close_button.clicked.connect(realtime_warning.hide.slot());
		realtime_warning.set_modal( true );
		realtime_warning.set_transient_for( *this );
		realtime_warning.show_all();
	}
#ifdef _DEBUG
	cout << "<GUI::GUI()> success" << endl;
#endif
}


void
GUI::realize_impl	( )
{
	Gtk::Window::realize_impl();
	
	about_pixmap = new Gtk::Pixmap( splash_xpm );
}

Gtk::MenuBar*
GUI::create_menus	( )
{
	using namespace Gtk::Menu_Helpers;
        using namespace Gtk;
	
	//
	// File menu
	//
        Menu *menu_file = manage (new Menu());
        MenuList& list_file = menu_file->items ();
	
	list_file.push_back (MenuElem("_Open Bank","<control>O",
			slot(this, &GUI::bank_open)));
//	list_file.push_back (MenuElem("_Save Bank","<control>S",
//			slot(this, &GUI::bank_save)));
	list_file.push_back (MenuElem("_Save Bank As...","<control>S",
			slot(this, &GUI::bank_save_as)));
	
	list_file.push_back (SeparatorElem());
	
	list_file.push_back (MenuElem("_Quit","<control>Q",
			bind(slot(this, &GUI::event_handler),"quit")));
	
	
	//
	// Preset menu
	//
        Menu *menu_preset = manage (new Menu());
        MenuList& list_preset = menu_preset->items ();
	
	list_preset.push_back (manage (new TearoffMenuItem ()));
	
	list_preset.push_back (MenuElem("_New","<control>N",
			slot(this, &GUI::preset_new)));
	list_preset.push_back (MenuElem("_Copy","<control>C",
			slot(this, &GUI::preset_copy)));
	list_preset.push_back (MenuElem("_Paste","<control>V",
			slot(this, &GUI::preset_paste)));
	list_preset.push_back (MenuElem("Paste as New","",
			slot(this, &GUI::preset_paste_as_new)));
	
	list_preset.push_back (SeparatorElem());
	
	list_preset.push_back (MenuElem("Rename","",
			bind(slot(this, &GUI::event_handler),
			"preset::rename")));
	list_preset.push_back (MenuElem("Clear","",
			bind(slot(this, &GUI::event_handler),
			"preset::delete")));

	list_preset.push_back (SeparatorElem());
	
	list_preset.push_back (MenuElem("_Randomise","<control>R",
			bind(slot(this, &GUI::event_handler),
			"preset::randomise")));

	list_preset.push_back (SeparatorElem());
	
	list_preset.push_back (MenuElem("Import...","",
			bind(slot(this, &GUI::event_handler),
			"preset::import")));
	list_preset.push_back (MenuElem("Export...","",
			bind(slot(this, &GUI::event_handler),
			"preset::export")));

			
	//
	// Config menu
	//
        Menu *menu_config = manage (new Menu());
        MenuList& list_config = menu_config->items ();

	list_config.push_back (MenuElem("Interface Font...","",
			bind(slot(this, &GUI::event_handler),"font")));
	list_config.push_back (MenuElem("MIDI Controllers...","<control>M",
			slot(this, &GUI::config_controllers)));
	
	
	//
	// Utils menu
	//
        Menu *menu_utils = manage (new Menu());
        MenuList& list_utils = menu_utils->items ();

	MenuItem *menu_item = manage (new MenuItem("Virtual Keyboard"));
	menu_item->activate.connect 
			(bind(slot(this, &GUI::event_handler),"vkeybd"));
	if (config->alsa_seq_client_id==0)
		menu_item->set_sensitive( false );
	// test for presence of vkeybd.
	if (command_exists ("vkeybd") != 0)
		menu_item->set_sensitive( false );
	list_utils.push_back (*menu_item);

	menu_item = manage (new MenuItem("Record to .wav file..."));
	menu_item->activate.connect 
			(bind(slot(this, &GUI::event_handler),"record_dialog"));
	if (audio_out)
		if (!audio_out->canRecord ())
			menu_item->set_sensitive (false);
	list_utils.push_back (*menu_item);
	
	list_utils.push_back (SeparatorElem());
	
	//
	// ALSA-MIDI sub-menu
	//
        Menu *menu_utils_midi = manage (new Menu());
        MenuList& list_utils_midi = menu_utils_midi->items ();
	
	menu_item = manage (new MenuItem("kaconnect"));
	menu_item->activate.connect 
			(bind(slot(this, &GUI::command_run),"kaconnect"));
	if (command_exists ("kaconnect") != 0)
		menu_item->set_sensitive( false );
	if (config->alsa_seq_client_id==0)
		menu_item->set_sensitive( false );
	list_utils_midi.push_back (*menu_item);
	
	menu_item = manage (new MenuItem("alsa-patch-bay"));
	menu_item->activate.connect 
			(bind(slot(this, &GUI::command_run),"alsa-patch-bay --driver alsa"));
	if (command_exists ("alsa-patch-bay") != 0)
		menu_item->set_sensitive( false );
	if (config->alsa_seq_client_id==0)
		menu_item->set_sensitive( false );
	list_utils_midi.push_back (*menu_item);
	
	list_utils.push_back (MenuElem("MIDI (ALSA) connections","",*menu_utils_midi));
	//
	// JACK sub-menu
	//
        Menu *menu_utils_jack = manage (new Menu());
        MenuList& list_utils_jack = menu_utils_jack->items ();
	
	menu_item = manage (new MenuItem("qjackconnect"));
	menu_item->activate.connect 
			(bind(slot(this, &GUI::command_run),"qjackconnect"));
	if (command_exists ("qjackconnect") != 0)
		menu_item->set_sensitive( false );
	if (config->audio_driver != "jack" && config->audio_driver != "JACK")
		menu_item->set_sensitive( false );
	list_utils_jack.push_back (*menu_item);
	
	menu_item = manage (new MenuItem("alsa-patch-bay"));
	menu_item->activate.connect 
			(bind(slot(this, &GUI::command_run),"alsa-patch-bay --driver jack"));
	if (command_exists ("alsa-patch-bay") != 0)
		menu_item->set_sensitive( false );
	if (config->audio_driver != "jack" && config->audio_driver != "JACK")
		menu_item->set_sensitive( false );
	list_utils_jack.push_back (*menu_item);
	
	list_utils.push_back (MenuElem("Audio (JACK) connections","",*menu_utils_jack));
	
	
	
	//
	// Menubar
	//
	MenuBar *menu_bar = manage (new MenuBar ());
	
	menu_bar->set_shadow_type( GTK_SHADOW_NONE );
	
	MenuList& list_bar = menu_bar->items();
	list_bar.push_back (MenuElem("_File","<alt>F",*menu_file));
	list_bar.push_back (MenuElem("_Preset","<alt>P",*menu_preset));
	list_bar.push_back (MenuElem("_Config","<alt>C",*menu_config));
	list_bar.push_back (MenuElem("_Utils","<alt>U",*menu_utils));
	
	
	menu_item = manage (new MenuItem("Analogue Modelling SYNTHesizer"));
	menu_item->right_justify();
	menu_item->activate.connect (
			bind(slot(this, &GUI::event_handler),"help::about"));
	list_bar.push_back (*menu_item);
	
	return menu_bar;
}


void
GUI::config_controllers()
{
    manage(new ControllerMapDialog( pipe[1], *midi_controller, *preset_controller ));
}



void
GUI::arrange()
{
	editor_panel->arrange ( );
	set_usize( 605, 450 );
}

void 
GUI::init()
{
	Preset *preset = &(preset_controller->getCurrentPreset());
	
	editor_panel = new EditorPanel (preset, pipe[1]);
	
	vbox.pack_start (*(create_menus ()));
	Gtk::HBox *tmphbox = manage (new Gtk::HBox());
	tmphbox->add (*(manage( new Gtk::Label () )));
	tmphbox->add (*presetCV);
	tmphbox->add (*(manage( new Gtk::Label () )));
	vbox.pack_start (*tmphbox,0,0);
	vbox.pack_start (*editor_panel,0,0);
	vbox.pack_start (statusBar);
	add (vbox);
	arrange();
	show_all ();
	
	
	presetCV->setPresetController(*preset_controller);
	
		
	char cstr[10];
	status = " Midi Driver: ";
	status += config->midi_driver;
	if( config->midi_driver == "OSS" ){
		status += ":";
		status += config->oss_midi_device;
	}
	status += "   Midi Channel: ";
	if( config->midi_channel ){
		sprintf( cstr, "%2d", config->midi_channel );
		status += string(cstr);
	} else status += "All";
	status += "   Audio Driver: ";
	status += config->audio_driver;
	if( config->audio_driver == "OSS" ){
		status += " : ";
		status += config->oss_audio_device;
	} else if( config->audio_driver == "ALSA" ){
		status += " : ";
		status += config->alsa_audio_device;
	}
	status += "   Sample Rate:";
	sprintf( cstr, "%d", config->sample_rate );
	status += string(cstr);
	status += " Hz   ";
	if( !config->realtime )
	{
		status += "Poly: ";
		sprintf( cstr, "%d", config->polyphony );
		status += string(cstr);
	}
	if(config->realtime)
		status += "   Realtime Priority: YES";
	else
		status += "   Realtime Priority: NO";
	statusBar.push( 1, status );
}









void 
GUI::event_handler(string text)
{
    if (text == "preset::randomise") {
		preset_controller->getCurrentPreset().randomise();
		return;
    } else if (text == "help::about") {
		about_window.show_all();
		return;
    } else if (text == "load") {
		return;
    } else if (text == "commit") {
		return;
    } else if (text == "preset::rename") {
		preset_rename_entry.set_text( 
			preset_controller->getCurrentPreset().getName() );
		preset_rename_entry.grab_focus();
		preset_rename.show_all();
		return;
    } else if (text == "preset::rename::ok") {
		preset_controller->getCurrentPreset().setName( 
			preset_rename_entry.get_text() );
		presetCV->update();
		preset_rename.hide();
		return;
    } else if (text == "preset::delete") {
		preset_delete.show_all();
		return;
    } else if (text == "preset::delete::ok") {
		preset_controller->deletePreset();
		preset_controller->commitPreset();
		preset_controller->selectPreset( 0 );
		presetCV->update();
		preset_delete.hide();
		return;
    } else if (text == "preset::export") {
		preset_export_dialog.show_all();
		return;
    } else if (text == "preset::export::ok") {
		string fn = preset_controller->getCurrentPreset().getName();
		fn += ".amSynthPreset";
		string file = preset_export_dialog.get_filename();
		file += fn;
		preset_controller->exportPreset( file );
		preset_export_dialog.hide();
		return;
    } else if (text == "preset::import") {
		preset_import_dialog.complete( "*.amSynthPreset" );
		preset_import_dialog.show_all();
		return;
    } else if (text == "preset::import::ok") {
		preset_controller->importPreset( preset_import_dialog.get_filename() );
		preset_import_dialog.hide();
		return;
    } else if (text == "quit") {
		quit_confirm.show_all();
		return;
    } else if (text == "quit::ok") {
		quit_confirm.hide_all();
		hide();
		return;
    } else if (text == "record_dialog" ) {
		record_dialog.show_all();
		return;
    } else if (text == "record_dialog::choose" ) {
		record_fileselect.show_all();
		return;
    } else if (text == "record::fileselect::ok" ) {
		record_entry.set_text( record_fileselect.get_filename() );
		record_fileselect.hide_all();
		return;
    } else if (text == "record_dialog::close" ) {
		audio_out->stopRecording();
		record_dialog.hide_all();
		return;
    } else if (text == "record_dialog::record" ) {
		if( record_recording == false )
		{
			audio_out->setOutputFile( record_entry.get_text() );
			audio_out->startRecording();
			record_recording = true;
			record_statusbar.pop( 1 );
			record_statusbar.push( 1, "capture status: RECORDING" );
		}
		return;
    } else if ( text == "record_dialog::pause" ) {
	    if( record_recording == true )
	    {
		    audio_out->stopRecording();
		    record_recording = false;
		    record_statusbar.pop( 1 );
		    record_statusbar.push( 1, "capture status: STOPPED" );
	    }
	    return;
    }
    else if (text=="vkeybd")
    {
	    string tmp = "vkeybd --addr ";
	    char tc[5];
	    sprintf( tc, "%d", config->alsa_seq_client_id );
	    tmp += string(tc);
	    tmp += ":0 &";
	    system( tmp.c_str() );
    } 
    else if (text=="font")
    {
	    font_sel.show_all ( );
    } 
    else if (text=="font::apply")
    {
	    set_x_font ( string(font_sel.get_font_name()).c_str() );
    } 
    else if (text=="font::ok")
    {
	    set_x_font ( string(font_sel.get_font_name()).c_str() );
	    font_sel.hide ( );
    } 
    else if (text=="font::cancel")
    {
	    font_sel.hide ( );
    } 
    else {
		cout << "no handler for event: " << text << endl;
		return;
    }
}

gint
GUI::idle_callback()
{
	if( config->active_voices != lnav )
	{
		string txt = status;
		txt += "   ";
		char cstr[3];
		sprintf( cstr, "%d", config->active_voices );
		txt += string( cstr );
		if( config->polyphony != 0 )
		{
			sprintf( cstr, "%d", config->polyphony );
			txt += "/";
			txt += string(cstr);
		}
		txt += " Voices Active";
		statusBar.pop( 1 );
		statusBar.push( 1, txt );
	}
	return true;
}

GUI::~GUI()
{
}

void
GUI::update()
{
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
}

void
GUI::preset_new		( )
{
	preset_controller->newPreset ();
}

void
GUI::preset_copy	( )
{
	clipboard_preset.clone (preset_controller->getCurrentPreset ());
}

void
GUI::preset_paste	( )
{
	preset_controller->getCurrentPreset().clone (clipboard_preset);
}

void
GUI::preset_paste_as_new( )
{
	preset_new ();
	preset_controller->getCurrentPreset().clone (clipboard_preset);
}

void
GUI::bank_open		( )
{
	d_bank_open.show_all ();
}

void
GUI::bank_open_ok	( )
{
	preset_controller->savePresets (config->current_bank_file.c_str ());
	config->current_bank_file = d_bank_open.get_filename ();
	preset_controller->loadPresets (config->current_bank_file.c_str ());
	d_bank_open.hide ();
}
/*
void
GUI::bank_save		( )
{
}
*/
void
GUI::bank_save_as	( )
{
	d_bank_save_as.show_all ();
}

void
GUI::bank_save_as_ok	( )
{
	config->current_bank_file = d_bank_save_as.get_filename ();
	preset_controller->savePresets (config->current_bank_file.c_str ());
	d_bank_save_as.hide ();
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
