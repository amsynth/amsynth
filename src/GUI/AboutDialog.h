/* amSynth
 * (c) 2002 Nick Dowell
 */

#ifndef _ABOUT_DIALOG_H
#define _ABOUT_DIALOG_H

#include <gtk--/dialog.h>
#include <gtk--/label.h>
#include <gtk--/button.h>
#include <gtk--/pixmap.h>

class AboutDialog : public Gtk::Dialog {
public:
	AboutDialog();
	gint delete_event_impl( GdkEventAny * ) { hide_all(); };
private:
	Gtk::Label about_label;
	Gtk::Pixmap *about_pixmap;
	Gtk::Button about_close_button;
};

#endif