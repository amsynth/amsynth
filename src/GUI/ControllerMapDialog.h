/* amSynth
 * (c) 2002,2003 Nick Dowell
 */
#ifndef _CONTROLLERMAPDIALOG_H
#define _CONTROLLERMAPDIALOG_H

#include "../UpdateListener.h"
#include "Request.h"

#include <gtk--/dialog.h>
#include <gtk--/label.h>
#include <gtk--/combo.h>
#include <gtk--/button.h>
#include <gtk--/menu.h>

class MidiController;
class PresetController;
class ControllerMapDialog : public Gtk::Window, public UpdateListener
{
public:
		ControllerMapDialog	( int pipe_d,
					MidiController & mc,
					PresetController & pc );
		~ControllerMapDialog	( );
    
	void	update			( );
	gint	popup_menu		( GdkEvent * event );
	void	midi_select_controller	( );
	void	select_controller	( int cc );
	void	select_parameter	( );
        gint	delete_event_impl	( GdkEventAny * ) 
			{ hide_all(); return 0; };
    
private:
	gboolean		supress_callback;
	int			piped;
    
	MidiController		*midi_controller;
	PresetController	*preset_controller;
	Request			request;

	Gtk::Menu		*m_menu_controllers;
	Gtk::Button		*m_button_controller;
	Gtk::Combo		*m_combo;
	Gtk::Label		*m_label_controller;
	
	Gtk::HBox		*hbox;
	Gtk::VBox		*vbox, *vboxl;
	int			m_cc;
};

#endif
