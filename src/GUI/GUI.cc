/* amSynth
 * (c) 2002 Nick Dowell
 */
#include "GUI.h"
#include <stdio.h>
#include <list>

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

GUI::GUI( Config & config, MidiController & mc, 
			VoiceAllocationUnit & vau, int pipe[2], GenericOutput & audio, const char *title )
{
#ifdef _DEBUG
	cout << "<GUI::GUI()>" << endl;
#endif
	lnav = -1;
	
	this->config = &config;
	this->midi_controller = &mc;
	this->pipe = pipe;
	this->vau = &vau;
	this->audio_out = &audio;
    hide.connect( Gtk::Main::quit.slot() );
	
	if(this->config->realtime)
		// messes up the audio thread's timing if not realtime...
		Gtk::Main::timeout.connect( slot(this,&GUI::idle_callback), 200 );

//    string title = "amSynth";
#ifdef _DEBUG
//    title += " DEBUG ( build: ";
//    title += __DATE__;
//    title += " @ ";
//    title += __TIME__;
//    title += ")";
#endif
#ifndef _DEBUG
//    title += VERSION;
#endif

    set_title( title );
    set_policy( false, false, false );

    active_param = 0;

    GtkShadowType frame_shadow = GTK_SHADOW_OUT;

	Gtk::Style *style = Gtk::Style::create();
	Gdk_Font font = style->get_font();
	font.load( "-*-helvetica-medium-r-*-*-*-100-*-*-*-*-*-*" );
	style->set_font( font );
	
	for (int i = 0; i < 31; i++)
	{
		parameterView[i] = new ParameterKnob( pipe[1] );
		parameterView[i]->set_style( *style );
	}
	for (int i = 0; i < 10; i++)
	{
		rb_pv[i] = new RadioButtonParameterView( pipe[1] );
		rb_pv[i]->set_style( *style );
	}
	param_switch = new ParameterSwitch( pipe[1] );
	param_switch->set_style( *style );
	
	osc1_frame.set_style( *style );
	osc2_frame.set_style( *style );
	osc_mix_frame.set_style( *style );
	reverb_frame.set_style( *style );
	distortion_frame.set_style( *style );
	filter_frame.set_style( *style );
	amp_frame.set_style( *style );
	mod_frame.set_style( *style );
	

#ifdef _DEBUG
	cout << "<GUI::GUI()> created ParameterViews" << endl;
#endif
    /* the menu
       how do menus work in GTK-- ?

       --+Gtk::MenuBar  (the bar at the top of the screen)
       |
       +--+Gtk::MenuItem    (eg "File")
       |
       +--+Gtk::Menu     (a container)
       |
       +--Gtk::MenuItem   (eg "Save")
       +--Gtk::MenuItem   (eg "Quit")
     */
	for(int i=0; i<30; i++)
		menu_item[i] = new Gtk::MenuItem;
	
	// the menu bar
	menu_bar.set_shadow_type( GTK_SHADOW_NONE );
	menu_bar.append( *menu_item[0] );
	menu_bar.append( *menu_item[10] );
	menu_bar.append( *menu_item[2] );
	menu_bar.append( *menu_item[29] );
	
	// the file menu
	menu_item[0]->add_label( "File" );
	menu_item[0]->set_submenu( file_menu );

	//file_menu.append( *menu_item[2] );
	menu_item[1]->add_label( "Quit" );
	menu_item[1]->activate.connect( 
		bind(slot(this, &GUI::event_handler),"quit"));
	menu_item[3]->add_label( "Capture output to file" );
	menu_item[3]->activate.connect( 
		bind(slot(this, &GUI::event_handler),"record_dialog"));
	menu_item[4]->add_label( "Launch Virtual Keybord" );
	menu_item[4]->activate.connect(
			bind(slot(this, &GUI::event_handler),"vkeybd"));
	
	
	//
	// grey out menu item if there is no recording interface present
	// 
	if (!audio_out->canRecord())
		menu_item[3]->set_sensitive( false );
	//
	// grey out virtual keyboard if we arent running alsa, or no vkeybd..
	// 
	if (config.alsa_seq_client_id==0)
		menu_item[4]->set_sensitive( false );
	// test for presence of vkeybd.
	int sys_rtn = system("vkeybd --help 2> /dev/null");
	if (WEXITSTATUS(sys_rtn)==127)	// exit code 127 = program not found
		menu_item[4]->set_sensitive( false );
	
	file_menu.append( *menu_item[3] );
	file_menu.append( *menu_item[4] );
	file_menu.append( *menu_item[1] );
	
	menu_item[2]->add_label( "Configure MIDI Controllers" );
	menu_item[2]->activate.connect( 
		bind(slot(this, &GUI::event_handler),"controller_map_dialog"));
	
	// the Preset menu
	menu_item[10]->add_label( "Preset" );
	menu_item[10]->set_submenu( preset_menu );
	preset_menu.append( preset_menu_tearoff );
	preset_menu.append( *menu_item[11] );
	menu_item[11]->add_label( "New" );
	menu_item[11]->activate.connect
		(bind(slot(this, &GUI::event_handler),"preset::new"));
	preset_menu.append( *menu_item[12] );
	menu_item[12]->add_label( "Rename" );
	menu_item[12]->activate.connect
		(bind(slot(this, &GUI::event_handler),"preset::rename"));
	preset_menu.append( *menu_item[13] );
	menu_item[13]->add_label( "Copy" );
	menu_item[13]->activate.connect
		(bind(slot(this, &GUI::event_handler),"preset::copy"));
	preset_menu.append( *menu_item[16] );
	menu_item[16]->add_label( "Save As..." );
	menu_item[16]->activate.connect
		(bind(slot(this, &GUI::event_handler),"preset::saveas"));
	preset_menu.append( *menu_item[14] );
	menu_item[14]->add_label( "Randomise" );
	menu_item[14]->activate.connect
		(bind(slot(this, &GUI::event_handler),"preset::randomise"));
	preset_menu.append( *menu_item[15] );
	menu_item[15]->add_label( "Delete" );
	menu_item[15]->activate.connect
		(bind(slot(this, &GUI::event_handler),"preset::delete"));
	preset_menu.append( *menu_item[18] );
	menu_item[18]->add_label( "Import as current" );
	menu_item[18]->activate.connect
		(bind(slot(this, &GUI::event_handler),"preset::import"));
	menu_item[17]->add_label( "Export current" );
	preset_menu.append( *menu_item[17] );
	menu_item[17]->activate.connect
		(bind(slot(this, &GUI::event_handler),"preset::export"));
	
	menu_item[29]->add_label("Analogue Modelling SYNTHesizer");
	menu_item[29]->activate.connect
		(bind(slot(this, &GUI::event_handler),"help::about"));
	menu_item[29]->right_justify();
	
#ifdef _DEBUG
	cout << "<GUI::GUI()> created menus" << endl;
#endif
	
	/*
	 * The main panel
	 */
	
    presetCV = new PresetControllerView( pipe[1], *this->vau );
#ifdef _DEBUG
	cout << "<GUI::GUI()> created presetCV" << endl;
#endif
    // oscillator 1 controls
    osc1_frame.set_shadow_type( frame_shadow );
    osc1_frame.set_label( "Oscillator 1" );
	osc1_fixed.put( *rb_pv[0], 0, 0 );				// waveform
	osc1_fixed.put( *parameterView[18], 10, 135 );	// pulsewidth
//    osc1_vbox.add(*rb_pv[0]);
//    osc1_vbox.add(*parameterView[18]);
    osc1_frame.add( osc1_fixed );

    // oscillator 2 controls
    osc2_frame.set_shadow_type( frame_shadow );
    osc2_frame.set_label( "Oscillator 2" );
	osc2_fixed.put( *param_switch, 85, 0 );			// osc. sync
	osc2_fixed.put( *rb_pv[1], 10, 0 );				// waveform
	osc2_fixed.put( *parameterView[1], 150, 70 );	// pulsewidth
	osc2_fixed.put( *rb_pv[2], 85, 30 );			// octave
	osc2_fixed.put( *parameterView[3], 150, 0 );	// detune
//	osc2_vbox.add(*param_switch);
//    osc2_vbox.add(*rb_pv[1]);
//    osc2_vbox.add(*parameterView[1]);
//    osc2_vbox.add(*rb_pv[2]);
//    osc2_vbox.add(*parameterView[3]);
    osc2_frame.add( osc2_fixed );

    // oscillator mixer section
    osc_mix_frame.set_shadow_type(frame_shadow);
    osc_mix_frame.set_label("Oscillator Mixer");
    osc_mix_vbox.add(*parameterView[4]);
	osc_mix_vbox.add(*rb_pv[3]);
    osc_mix_frame.add(osc_mix_vbox);

    // filter controls
    filter_frame.set_shadow_type(frame_shadow);
    filter_frame.set_label("Low Pass Filter");
    filter_hbox1.add(*parameterView[6]);
    filter_hbox1.add(*parameterView[7]);
//    filter_hbox1.add(*parameterView[12]);
	filter_hbox1.add(*parameterView[5]);
    filter_vbox.add(filter_hbox1);
    filter_hbox2.add(*parameterView[8]);
    filter_hbox2.add(*parameterView[9]);
    filter_hbox2.add(*parameterView[10]);
    filter_hbox2.add(*parameterView[11]);
    filter_vbox.add(filter_hbox2);
    filter_frame.add(filter_vbox);

    // amplifier controls
    amp_frame.set_shadow_type(frame_shadow);
    amp_frame.set_label("Amplifier");
//    amp_hbox1.add(*parameterView[17]);
//    amp_hbox1.add(*parameterView[19]);
//    amp_vbox.add(amp_hbox1);
    amp_hbox2.add(*parameterView[13]);
    amp_hbox2.add(*parameterView[14]);
    amp_hbox2.add(*parameterView[15]);
    amp_hbox2.add(*parameterView[16]);
    amp_vbox.add(amp_hbox2);
    amp_frame.add(amp_vbox);

    // mod controls
    mod_frame.set_shadow_type( frame_shadow );
    mod_frame.set_label( "Modulation" );
	mod_frame.add( mod_hbox );
    mod_hbox.add( *parameterView[20] );		// lfo rate
    mod_hbox.add( *rb_pv[4] );				// waveform switch
	mod_hbox.add( *parameterView[22] );		// freq mod
	mod_hbox.add( *parameterView[12] );		// filter mod
	mod_hbox.add( *parameterView[17] );		// amp mod
//	mod_hbox.add( *parameterView[30] );		// osc1 pwm mod

    // reverb :)) section
    reverb_frame.set_shadow_type( frame_shadow );
    reverb_frame.set_label( "Reverb" );
	reverb_hbox.add( *parameterView[24] );	// amount
    reverb_hbox.add( *parameterView[23] );	// room size
//    reverb_hbox.add( *parameterView[24] );	// amount
//    reverb_hbox.add( *parameterView[25] );	// dry
    reverb_hbox.add( *parameterView[26] );	// width
    reverb_hbox.add( *parameterView[28] );	// damping
    reverb_frame.add( reverb_hbox );

	// distortion section
	distortion_frame.set_shadow_type(frame_shadow);
	distortion_frame.set_label("Distortion");
//	distortion_hbox.add(*parameterView[29]);
	distortion_hbox.add(*parameterView[30]);
	distortion_frame.add(distortion_hbox);

    // the main panel
    main_panel.put(osc1_frame, 0, 0);
    main_panel.put(osc2_frame, 0, 0);
    main_panel.put(osc_mix_frame, 0, 0);
    main_panel.put(filter_frame, 0, 0);
    main_panel.put(amp_frame, 0, 0);
    main_panel.put(mod_frame, 0, 0);
//    main_panel.put(pitch_frame, 0, 0);
    main_panel.put(reverb_frame, 0, 0);
	main_panel.put(*presetCV, 0, 0);
	main_panel.put( *parameterView[19], 0, 0 );
	main_panel.put(distortion_frame, 0, 0);
	
    // the main window
    vbox.pack_start(menu_bar);
    vbox.pack_start(main_panel);
    vbox.pack_start(statusBar);
#ifdef _DEBUG
	cout << "<GUI::GUI()> put all controls" << endl;
#endif
    add( vbox );
    show_all();

	// the preset rename dialog
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

	// the new preset dialog
	preset_new.set_title( "Create a New Preset" );
	preset_new.set_usize( 300, 200 );
	preset_new.get_vbox()->add( preset_new_label );
	preset_new_label.set_text( "Enter new Preset Name:" );
	preset_new.get_vbox()->add( preset_new_entry );
	preset_new.get_action_area()->add( preset_new_ok );
	preset_new_ok.add_label( "Confirm", 0.5, 0.5 );
	preset_new_ok.clicked.connect(
		bind(slot(this, &GUI::event_handler),"preset::new::ok"));
	preset_new.get_action_area()->add( preset_new_cancel );
	preset_new_cancel.clicked.connect( preset_new.hide.slot() );
	preset_new_cancel.add_label( "Cancel", 0.5, 0.5 );
	preset_new.set_modal( true );
	preset_new.set_transient_for( *this );
	preset_new.delete_event.connect( 
		bind( slot( this, &GUI::delete_events ), &preset_new ) );
	
	// the copy preset dialog
	preset_copy.set_title( "Copy another Preset" );
	preset_copy.set_usize( 300, 200 );
	preset_copy.get_vbox()->add( preset_copy_label );
	preset_copy_label.set_text( "Select preset to copy parameters from" );
	preset_copy.get_vbox()->add( preset_copy_combo );
	preset_copy.get_action_area()->add( preset_copy_ok );
	preset_copy_ok.add_label( "Copy", 0.5, 0.5 );
	preset_copy_ok.clicked.connect(
		bind(slot(this, &GUI::event_handler),"preset::copy::ok"));
	preset_copy.get_action_area()->add( preset_copy_cancel );
	preset_copy_cancel.add_label( "Cancel", 0.5, 0.5 );
	preset_copy_cancel.clicked.connect( preset_copy.hide.slot() );
	preset_copy.set_modal( true );
	preset_copy.set_transient_for( *this );
	preset_copy.delete_event.connect( 
		bind( slot( this, &GUI::delete_events ), &preset_copy ) );

	// the saveas preset dialog
	preset_saveas.set_title( "Save current settings as.." );
	preset_saveas.set_usize( 300, 200 );
	preset_saveas.get_vbox()->add( preset_saveas_label );
	preset_saveas_label.set_text( "Choose a preset name to\nsave these settings as..." );
	preset_saveas.get_vbox()->add( preset_saveas_entry );
	preset_saveas.get_action_area()->add( preset_saveas_ok );
	preset_saveas_ok.add_label( "Save", 0.5, 0.5 );
	preset_saveas_ok.clicked.connect(
		bind(slot(this, &GUI::event_handler),"preset::saveas::ok"));
	preset_saveas.get_action_area()->add( preset_saveas_cancel );
	preset_saveas_cancel.add_label( "Cancel", 0.5, 0.5 );
	preset_saveas_cancel.clicked.connect(
		bind(slot(this, &GUI::event_handler),"preset::saveas::cancel"));
	preset_saveas.set_modal( true );
	preset_saveas.set_transient_for( *this );
	preset_saveas.delete_event.connect( 
		bind( slot( this, &GUI::delete_events ), &preset_saveas ) );

	// the delete preset dialog
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
	
	// the about window
	about_window.set_title( "About" );
	about_window.get_vbox()->add( *about_pixmap );
	about_window.get_action_area()->add( about_close_button );
	about_close_button.add_label( "sweet", 0.5, 0.5 );
	about_close_button.clicked.connect( about_window.hide.slot() );
	about_window.set_transient_for( *this );
	about_window.delete_event.connect( 
		bind( slot( this, &GUI::delete_events ), &about_window ) );
	
	// export dialog
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
		
	// import dialog
	preset_import_dialog.set_title( "Import as current preset" );
	preset_import_dialog.get_cancel_button()->clicked.connect(
		preset_import_dialog.hide.slot() );
	preset_import_dialog.get_ok_button()->clicked.connect(
		bind(slot(this, &GUI::event_handler),"preset::import::ok"));
	preset_import_dialog.set_modal( true );
	preset_import_dialog.set_transient_for( *this );
	preset_import_dialog.delete_event.connect( 
		bind( slot( this, &GUI::delete_events ), &preset_import_dialog ) );
	
	// record file-selector dialog
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
	
	// the record dialog
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
	
	
	// the quit confirmation window
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
	
	// show realtime warning message if necessary
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
GUI::realize_impl()
{
	Gtk::Window::realize_impl();
	
	about_pixmap = new Gtk::Pixmap( splash_xpm );
	
    GdkPixmap *pixmap;
    GdkBitmap *bitmap;
#ifdef _DEBUG
	cout << "<GUI::realize_impl()> creating knob pixmap" << endl;
#endif
    pixmap = gdk_pixmap_create_from_xpm_d(get_window(), &bitmap,
		get_style()->gtkobj()->bg, knob_xpm);
#ifdef _DEBUG
	cout << "<GUI::realize_impl()> initialising Knobs" << endl;
#endif
	for(int i=0; i < 31; i++)
		parameterView[i]->setPixmap( pixmap, 50, 50, 15 );
}

void
GUI::arrange()
{
    gint x, y;
    x = 10;
    y = 35;
    main_panel.move( osc1_frame, x, y );
	y += 220;
    main_panel.move( osc2_frame, x, y );
    x += 75;
	y += 80;
    main_panel.move( osc_mix_frame, x, 35 );
	main_panel.move( mod_frame, 240, y-80 );
	main_panel.move( distortion_frame, 530, y-80 );
	x = 175;
	y = 35;
    main_panel.move( filter_frame, x, y );
	x += 210;
    main_panel.move( amp_frame, x, y );
	y += 100;
	main_panel.move( reverb_frame, x, y );
	main_panel.move( *presetCV, 100, 5 );
	main_panel.resize_children();
	main_panel.move( *parameterView[19], 530, 340);		// master vol.
	
	set_usize( 605, 450 );
}

void 
GUI::init()
{
    Preset *preset = &(preset_controller->getCurrentPreset());

    /*
     * connect all the controls to their respective parameters.
     * this is not done in the constructor since we must have been given
     * the PresetController first..
     */

    // controls for oscillator 1
//    parameterView[0]->setParameter(preset->getParameter("osc1_waveform"));
    parameterView[18]->setParameter(preset->getParameter("osc1_pulsewidth"));
	parameterView[18]->drawValue( false );
	parameterView[18]->setName( "Pulsewidth" );
	
    rb_pv[0]->setParameter(preset->getParameter("osc1_waveform"));
    rb_pv[0]->setName("waveform");
    rb_pv[0]->setDescription(0, "random");
    rb_pv[0]->setDescription(1, "noise");
    rb_pv[0]->setDescription(2, "saw");
    rb_pv[0]->setDescription(3, "square");
    rb_pv[0]->setDescription(4, "sine");
	
    // controls for oscillator 2
    parameterView[1]->setParameter(preset->getParameter("osc2_pulsewidth"));
	parameterView[1]->drawValue( false );
	parameterView[1]->setName( "Pulsewidth" );
	
    rb_pv[1]->setParameter(preset->getParameter("osc2_waveform"));
    rb_pv[1]->setName("Waveform");
    rb_pv[1]->setDescription(0, "random");
    rb_pv[1]->setDescription(1, "noise");
    rb_pv[1]->setDescription(2, "saw");
    rb_pv[1]->setDescription(3, "square");
    rb_pv[1]->setDescription(4, "sine");
//      parameterView[2]->setParameter(preset->getParameter("osc2_range"));
    rb_pv[2]->setParameter(preset->getParameter("osc2_range"));
    rb_pv[2]->setName("octave");
    rb_pv[2]->setDescription(0, "+2");
    rb_pv[2]->setDescription(1, "+1");
    rb_pv[2]->setDescription(2, "0");
    rb_pv[2]->setDescription(3, "-1");
    parameterView[3]->setParameter(preset->getParameter("osc2_detune"));
	parameterView[3]->setName( "Detune" );
	parameterView[3]->drawValue( false );

    // oscillator mixer section controls
    parameterView[4]->setParameter(preset->getParameter("osc_mix"));
	parameterView[4]->drawValue( false );
    parameterView[4]->setName("osc1  -  osc2");
//      parameterView[5]->setParameter(preset->getParameter("osc_mix_mode"));
    rb_pv[3]->setParameter(preset->getParameter("osc_mix_mode"));
    rb_pv[3]->setName("Mix mode");
    rb_pv[3]->setDescription(0, "ring mod");
    rb_pv[3]->setDescription(1, "normal");

    // filter controls
	parameterView[5]->setParameter(preset->getParameter("filter_env_amount"));
	parameterView[5]->setName( "Env.\nAmount" );
    parameterView[6]->setParameter(preset->getParameter("filter_cutoff"));
	parameterView[6]->setName( "Cutoff\nFreq." );
	parameterView[6]->drawValue( false );
    parameterView[7]->setParameter(preset->getParameter("filter_resonance"));
	parameterView[7]->setName( "\nResonance" );
	parameterView[7]->drawValue( false );
    parameterView[8]->setParameter(preset->getParameter("filter_attack"));
	parameterView[8]->setName( "Attack" );
	parameterView[8]->drawValue( true );
    parameterView[9]->setParameter(preset->getParameter("filter_decay"));
	parameterView[9]->setName( "Decay" );
	parameterView[9]->drawValue( true );
    parameterView[10]->setParameter(preset->getParameter("filter_sustain"));
	parameterView[10]->setName( "Sustain" );
	parameterView[10]->drawValue( true );
    parameterView[11]->setParameter(preset->getParameter("filter_release"));
	parameterView[11]->setName( "Release" );
	parameterView[11]->drawValue( true );
    parameterView[12]->setParameter(preset->getParameter("filter_mod_amount"));
	parameterView[12]->setName( "Filter\nMod.\nAmount" );
	parameterView[12]->drawValue( false );


    // voice amplitude
    parameterView[13]->setParameter(preset->getParameter("amp_attack"));
	parameterView[13]->setName( "Attack" );
	parameterView[13]->drawValue( true );
    parameterView[14]->setParameter(preset->getParameter("amp_decay"));
	parameterView[14]->setName( "Decay" );
	parameterView[14]->drawValue( true );
    parameterView[15]->setParameter(preset->getParameter("amp_sustain"));
	parameterView[15]->setName( "Sustain" );
	parameterView[15]->drawValue( true );
    parameterView[16]->setParameter(preset->getParameter("amp_release"));
	parameterView[16]->setName( "Release" );
	parameterView[16]->drawValue( true );
    parameterView[17]->setParameter(preset->getParameter("amp_mod_amount"));
	parameterView[17]->setName( "Amplitude\nMod.\nAmount" );
	parameterView[17]->drawValue( false );
    /* 18 is taken */
    parameterView[19]->setParameter(preset->getParameter("master_vol"));
	parameterView[19]->setName( "Master Vol." );

    // mod section
    parameterView[20]->setParameter(preset->getParameter("lfo_freq"));
	parameterView[20]->drawValue( true );
	parameterView[20]->setName( "Rate" );
//      parameterView[21]->setParameter(preset->getParameter("lfo_waveform"));
    rb_pv[4]->setParameter(preset->getParameter("lfo_waveform"));
    rb_pv[4]->setName("waveform");
    rb_pv[4]->setDescription(0, "random");
    rb_pv[4]->setDescription(1, "noise");
    rb_pv[4]->setDescription(2, "saw");
    rb_pv[4]->setDescription(3, "square");
    rb_pv[4]->setDescription(4, "sine");

    // freq control section
    parameterView[22]->setParameter(preset->getParameter("freq_mod_amount"));
	parameterView[22]->setName( "Frequency\nMod.\nAmount" );
	parameterView[22]->drawValue( false );

    // reverb control section
    parameterView[23]->setParameter(preset->getParameter("reverb_roomsize"));
	parameterView[23]->setName( "Room\nSize" );
    parameterView[24]->setParameter(preset->getParameter("reverb_wet"));
	parameterView[24]->setName( "\nAmount" );
//    parameterView[25]->setParameter(preset->getParameter("reverb_dry"));
//	parameterView[25]->setName( "Dry" );
    parameterView[26]->setParameter(preset->getParameter("reverb_width"));
	parameterView[26]->setName( "Stereo\nWidth" );
//    parameterView[27]->setParameter(preset->getParameter("reverb_mode"));
//	parameterView[27]->setName( "Mode" );
    parameterView[28]->setParameter(preset->getParameter("reverb_damp"));
	parameterView[28]->setName( "\nDamping" );

    // distortion control section
//    parameterView[29]->setParameter(preset->getParameter("distortion_drive"));
//	parameterView[29]->setName( "Drive" );
	parameterView[30]->setParameter(preset->getParameter("distortion_crunch"));
	parameterView[30]->setName( "Crunch" );
	
//	parameterView[30]->setParameter(preset->getParameter("osc1_pwm_amount"));
	
	param_switch->setParameter(preset->getParameter("osc2_sync"));
	param_switch->setName( "Sync. to\nOsc. 1" );

    presetCV->setPresetController(*preset_controller);
	
    arrange();
	
	// MIDI Controllers dialog
	controller_map_dialog = new ControllerMapDialog( pipe[1], 
						*midi_controller, *preset_controller );

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
    } else if (text == "preset::new") {
		preset_new_entry.set_text( "" );
		preset_new_entry.grab_focus();
		preset_new.show_all();
		return;
    } else if (text == "preset::new::ok") {
		if(!preset_controller->newPreset()){
			preset_controller->getCurrentPreset().setName( 
				preset_new_entry.get_text() );
		}
		presetCV->update();
		preset_new.hide();
		return;
    } else if (text == "preset::copy") {
		// populate the combo box with the possible presets to copy from
		list<string> gl;
		for (int preset=0; preset<PRESETS; preset++){
			string preset_name = preset_controller->getPreset(preset).getName();
			if ( preset_name != "New Preset" ) gl.push_back( preset_name );
		}
		preset_copy_combo.set_popdown_strings( gl );
		preset_copy_combo.get_entry()->grab_focus();
		preset_copy.show_all();
		return;
    } else if (text == "preset::copy::ok") {
		string name = preset_controller->getCurrentPreset().getName();
		preset_controller->getCurrentPreset().clone( 
			preset_controller->getPreset( preset_copy_combo.get_entry()->get_text() ) );
		preset_controller->getCurrentPreset().setName( name );
		presetCV->update();
		preset_copy.hide();
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
    } else if (text == "preset::saveas") {
		preset_saveas_entry.grab_focus();
		preset_saveas.show_all();
		return;
    } else if (text == "preset::saveas::ok") {
		Preset preset;
		preset.clone( preset_controller->getCurrentPreset() );
		preset_controller->newPreset();
		preset_controller->getCurrentPreset().clone( preset );
		preset_controller->getCurrentPreset().setName( 
			preset_saveas_entry.get_text() );
		preset_controller->commitPreset();
		presetCV->update();
		preset_saveas.hide();
	} else if (text == "preset::saveas::cancel") {
		preset_saveas.hide();
	} else if (text == "controller_map_dialog") {
		controller_map_dialog->show_all();
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
		return true;
	}
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
