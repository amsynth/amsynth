/* a GTK-- Knob widget
 * (c) 2002-2005 Nick Dowell
 */

#ifndef _KNOB_H
#define _KNOB_H

#include <gtkmm/misc.h>
#include <gtkmm/main.h>
#include <gtkmm/style.h>
#include <gtkmm/adjustment.h>
 
// the knob turns from 30 degrees to 330 degrees, so graphics should fit that..

class Knob : public Gtk::Misc
{
public:
	Knob();
	~Knob();

	void setFrames(const Glib::RefPtr<Gdk::Pixbuf>&, int x, int y, int frames);

	void set_adjustment( Gtk::Adjustment & adjustment );
	void adjUpdate( Gtk::Adjustment * _adj );
	
protected:
	virtual bool on_expose_event			(GdkEventExpose*);
	virtual bool on_button_press_event		(GdkEventButton*);
	virtual bool on_button_release_event	(GdkEventButton*);
	virtual bool on_motion_notify_event		(GdkEventMotion*);
  
private:
	void mouse_pos_change					(gint x, gint y);
	bool pixmap_loaded;
	Glib::RefPtr<Gdk::Pixbuf> pixmap;
	GdkBitmap *bitmap;
	bool				myadj;
	Gtk::Adjustment		*adj;
	gint widget_x, widget_y;
	gint frame, width, height, frames, center_x, center_y;
};

#endif
