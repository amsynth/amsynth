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

class ControllerMapDialog : Gtk::Window
{
public:
	ControllerMapDialog();
	~ControllerMapDialog(){};
	void callback( string str ){};
	void show_all() { Gtk::Window::show_all(); };
	gint delete_event_impl( GdkEventAny * ) { Gtk::Widget::hide_all(); };
private:
	Gtk::Table table;
	Gtk::Label label[32];
	Gtk::Combo combo[32];
	MidiController *midi_controller;
};

#endif