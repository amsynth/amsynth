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

/**
 * A GTK-- "Knob" widget
 *
 * A GTK-- widget which implements a "knob" which can be controlled with the
 * mouse by the user. The appearance of the knob is dictated by the graphic
 * set with setPixmap(). Similar in operation to the Gtk::Scale widgets.
 */
class Knob : public Gtk::Misc
{
public:
	Knob();
	~Knob();
	/**
	 * Set the graphic to be used for the knob. The knob is set to turn from
	 * 30 to 330 degrees, so grphics must be created accordingly. The graphic
	 * should contain numerous image frames for the knob as it turns through 
	 * it's range. The number of frames is not fixed.
	 * @param pix The graphic to use for displaying the knob.
	 * @param x The width of each knob graphic frame.
	 * @param y The height of each knob graphic frame.
	 * @param frames The number of frames contained in pix.
	 */
	void setPixmap( GdkPixmap *pix, gint x, gint y, gint frames );
	/**
	 * Set the Gtk::Adjustment to use. This is used to hold the value 
	 * manipulated by the widget, in the same way that a Gtk::Scale widget
	 * works. (see GTK-- API reference documentation).
	 */
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
