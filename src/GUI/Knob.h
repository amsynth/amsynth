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
	virtual bool on_expose_event			(GdkEventExpose*);
	virtual bool on_button_press_event		(GdkEventButton*);
	virtual bool on_button_release_event	(GdkEventButton*);
	virtual bool on_motion_notify_event		(GdkEventMotion*);
  
private:
	void mouse_pos_change					(gint x, gint y);
	bool pixmap_loaded;
	GdkPixmap *pixmap;
	GdkBitmap *bitmap;
	bool				myadj;
	Gtk::Adjustment		*adj;
	gint widget_x, widget_y;
	gint frame, width, height, frames, center_x, center_y;
};

#endif
