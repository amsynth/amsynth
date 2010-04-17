/* amsynth
 * a GTK-- Knob widget
 * (c) 2002-2005 Nick Dowell
 */
#include "Knob.h"
#include <math.h>
#include <iostream>

using namespace std;

Knob::Knob()
:	adj(new Gtk::Adjustment(0.0, 0.0, 1.0, 0.01, 1.0, 0))
,	frame (0)
{
	set_events (Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
	set_double_buffered(true);
}

Knob::~Knob()
{
}

void
Knob::setFrames(const Glib::RefPtr<Gdk::Pixbuf>& pix, int x, int y, int frames)
{
	width = x;
	center_x = (gint)(x/2);
	height = y;
	center_y = (gint)(y/2);
	this->frames = frames;
	set_size_request (x, y);
	pixmap = pix;
}

void
Knob::set_adjustment(Gtk::Adjustment* adj)
{
	adj->signal_value_changed().connect(mem_fun(*this, &Knob::on_adj_value_changed));
	on_adj_value_changed();
}

void
Knob::on_adj_value_changed()
{
	int newFrame = (int)(-0.5f + frames *
						 (adj->get_value() - adj->get_lower()) /
						 (adj->get_upper() - adj->get_lower()) );
	
	if (frame != newFrame)
	{
		frame = newFrame;
		queue_draw();
	}
}

bool
Knob::on_expose_event			(GdkEventExpose *ev)
{
	if (pixmap && ev)
	{
		gdk_pixbuf_render_to_drawable(pixmap->gobj(), ev->window,
									  get_style()->get_black_gc()->gobj(),
									  get_width(), get_height()*frame,
									  0, 0, get_width(), get_height(),
									  GDK_RGB_DITHER_NONE, 0, 0);
	}
	return TRUE;
}

bool
Knob::on_button_press_event		(GdkEventButton *ev)
{
	widget_x = (gint)(ev->x_root - ev->x);
	widget_y = (gint)(ev->y_root - ev->y);
	
	gint x, y; GdkModifierType modifiers;
	gdk_window_get_pointer(ev->window, &x, &y, &modifiers);
	
	switch(ev->button){
		case 1:
			if (modifiers & GDK_CONTROL_MASK) {
				adj->set_value((adj->get_lower() + adj->get_upper()) / 2);
			} else {
				add_modal_grab ();
				mouse_pos_change( (gint)ev->x_root, (gint)ev->y_root );
			}
			break;
		case 4: // mouse wheel up
			adj->set_value( (adj->get_value()+adj->get_step_increment()) );
			break;
		case 5: // mouse wheel down
			adj->set_value( (adj->get_value()-adj->get_step_increment()) );
			break;
		default:
//			cout << "Knob: " << adj->get_value() << endl;
			break;
	}
	return TRUE;
}

bool
Knob::on_button_release_event	(GdkEventButton *)
{
	if(has_grab()) remove_modal_grab ();
	return TRUE;
}

bool
Knob::on_motion_notify_event	(GdkEventMotion *ev) 
{
	if(has_grab()) mouse_pos_change( (gint)ev->x_root, (gint)ev->y_root );
	return TRUE;
}

void
Knob::mouse_pos_change			(gint x_abs, gint y_abs)
{
	gfloat x = x_abs - ( widget_x + center_x );
	gfloat y = y_abs - ( widget_y + center_y );

	gfloat angle = atan(y/x);
  
	// map angle to range [0-2*PI] (0-360 degrees)
	if(x<0) angle += M_PI;
	else if(y <= 0) angle += 2*M_PI;
  
	// map so 0 degrees/rads is at the bottom of the dial
	angle -= M_PI/2;
	if(x>0 && y>0) angle += 2*M_PI;

	// map to range 30deg - 330 deg
	if(angle<M_PI/6) angle=M_PI/6;
	else if(angle>11*M_PI/6) angle=M_PI*11/6;

	// set adjustment value accordingly
	float val = (angle-M_PI/6)/(M_PI*10/6);
	val *= (adj->get_upper()-adj->get_lower());
	val += (adj->get_lower());
	
	adj->set_value(val);
}
