/* amSynth
 * (c) 2002,2003 Nick Dowell
 */

#include "ControllerMapDialog.h"

#include "../MidiController.h"
#include "../PresetController.h"
#include "controllers.h"
#include <gtkmm/menuitem.h>
#include <list>
#include <string>
#include <stdio.h>
#include <iostream>
#include <unistd.h>

using sigc::slot;
using sigc::bind;
using std::cout;

ControllerMapDialog::ControllerMapDialog
		( int pipe_d, MidiController & mc, PresetController & pc )
{
	piped = pipe_d;
	midi_controller = &mc;
	preset_controller = &pc;
	m_cc = 0;
	
	set_title( "MIDI Controller Config" );
	set_resizable (false);
	
	m_label_controller = manage( new Gtk::Label("Nothing") );

	m_button_controller = manage( new Gtk::Button() );

	m_button_controller->add(*m_label_controller);


	m_button_controller->signal_event().connect( mem_fun(this, &ControllerMapDialog::popup_menu) );

	m_menu_controllers = manage( new Gtk::Menu());

	// temp
	
	char b[20];
	
	// create control change
	
	for ( int i=0; i<8; i++ )
	{
		sprintf( b, "Controls %d-%d", (i*16), (i*16)+15 );
		Gtk::Menu *menu_cc = manage( new Gtk::Menu() ); 
	
		for( int j=0; j<16; j++ )
		{
			menu_cc->items().push_back( Gtk::Menu_Helpers::MenuElem( c_controller_names[i*16+j], 
                        bind(mem_fun(*this,&ControllerMapDialog::select_controller), i*16+j)));
		}
		m_menu_controllers->items().push_back(Gtk::Menu_Helpers::MenuElem( std::string(b) , *menu_cc ));
	}

	std::list<std::string> gl;
	gl.push_back( "null" );
	for (guint p=0; p<preset_controller->getCurrentPreset().ParameterCount(); p++)
	{
		string p_name = preset_controller->getCurrentPreset().getParameter(p).getName();
		if ( p_name != "unused" ) gl.push_back( p_name );
	}
	
	m_combo = manage (new Gtk::Combo());

	m_combo->set_popdown_strings( gl );
	m_combo->get_entry()->set_editable( false );
	m_combo->get_entry()->signal_changed().connect(
            mem_fun(*this, &ControllerMapDialog::select_parameter));

	request.slot = mem_fun(*this, &ControllerMapDialog::midi_select_controller );
	
	midi_controller->getLastControllerParam().addUpdateListener( *this );
	
	hbox = manage( new Gtk::HBox());
	vboxl = manage( new Gtk::VBox());
	vbox = manage( new Gtk::VBox());
	
	hbox->set_spacing( 10 );
	vboxl->set_spacing( 10 );
	vbox->set_spacing( 10 );
	
	vbox->add( *(manage( new Gtk::Label () )) );
	vbox->pack_start(*m_button_controller);
	vbox->pack_start(*m_combo);
	vbox->add( *(manage( new Gtk::Label () )) );
	
	vboxl->add (*(manage( new Gtk::Label () )));
	vboxl->add (*(manage( new Gtk::Label ("MIDI controller:",1) )));
	vboxl->add (*(manage( new Gtk::Label ("amSynth control:",1) )));
	vboxl->add (*(manage( new Gtk::Label () )));
	
	hbox->add( *(manage( new Gtk::Label () )) );
	hbox->add( *vboxl );
	hbox->add( *vbox );
	hbox->add( *(manage( new Gtk::Label () )) );
	add( *hbox );
	
	show_all();
}


ControllerMapDialog::~ControllerMapDialog()
{
	midi_controller->getLastControllerParam().removeUpdateListener(*this);
}

void
ControllerMapDialog::update()
{
	if( ::write( piped, &request, sizeof(request) ) != sizeof(request) )
		cout << "ParameterSwitch: error writing to pipe" << endl;
}

gint
ControllerMapDialog::popup_menu(GdkEvent *event)
{
	if (event->type == GDK_BUTTON_PRESS)
	{
		GdkEventButton *bevent = (GdkEventButton *) event; 
		m_menu_controllers->popup(bevent->button, bevent->time);

		// Tell calling code that we have handled this event; the buck
		// stops here.
		return true;
	}

	// Tell calling code that we have not handled this event; pass it on.
	return false;
}

void
ControllerMapDialog::midi_select_controller()
{
	int last_active = (int)midi_controller->getLastControllerParam().getValue();
	select_controller(last_active);
}

void
ControllerMapDialog::select_parameter()
{
	if(!supress_callback) midi_controller->setController(
		m_cc, preset_controller->getCurrentPreset().getParameter(
		m_combo->get_entry()->get_text())
	);
}

void 
ControllerMapDialog::select_controller( int cc )
{
	supress_callback = true;
	m_label_controller->set_text(c_controller_names[cc]);
	m_combo->get_entry()->set_text( 
            midi_controller->getController(cc).getName() );
	m_cc = cc;
	supress_callback = false;
}
