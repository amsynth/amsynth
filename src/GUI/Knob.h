/* a GTK-- Knob widget
 * (c) 2002 Nick Dowell
 */

#ifndef _KNOB_H
#define _KNOB_H

#include <gtk--/misc.h>
#include <gtk--/main.h>
#include <gtk--/style.h>
#include <gtk--/adjustment.h>
 
// the knob turns from 30 degrees to 330 degrees, so graphics should fit that..

class Knob : public Gtk::Misc
{
public:
	Knob();
	~Knob();	
	void setPixmap( GdkPixmap *pix, gint x, gint y, gint frames );
	void set_adjustment( Gtk::Adjustment & adjustment );
	void adjUpdate( Gtk::Adjustment * _adj );
	
protected:
	virtual void realize_impl();
	virtual gint expose_event_impl(GdkEventExpose*);
	virtual gint button_press_event_impl(GdkEventButton*);
	virtual gint button_release_event_impl(GdkEventButton*);
	virtual gint motion_notify_event_impl(GdkEventMotion*);
  
private:
	void redraw();
	void mouse_pos_change(gint x, gint y);
	bool pixmap_loaded;
	GdkPixmap *pixmap;
	GdkBitmap *bitmap;
	Gtk::Adjustment *adj;
	gint widget_x, widget_y;
	gint frame, width, height, frames, center_x, center_y, myadj;
};

#endif
