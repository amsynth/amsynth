/* amSynth
 * (c) 2002 Nick Dowell
 */
#ifndef _CONTROLLERMAPDIALOG_H
#define _CONTROLLERMAPDIALOG_H

#include "../MidiController.h"
#include <gtk--/window.h>
#include <gtk--/table.h>
#include <gtk--/label.h>
#include <gtk--/combo.h>

class ControllerMapDialog : public Gtk::Window
{
public:
	ControllerMapDialog( MidiController & mc, PresetController & pc );
	~ControllerMapDialog(){};
	void callback( gint cc );
	void _update_();
	gint delete_event_impl( GdkEventAny * ) { Gtk::Widget::hide_all(); };
private:
	Gtk::Table table;
	Gtk::Label label[32];
	Gtk::Combo combo[32];
	gboolean supress_callback;
	MidiController *midi_controller;
	PresetController *preset_controller;
};

#endif