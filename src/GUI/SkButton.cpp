/*
 *  SkButton.h
 *  amsynth
 *  (c) 2006 Nick Dowell
 */

#include "SkButton.h"

SkButton::SkButton()
:	mState(0)
{	
	set_events(Gdk::BUTTON_PRESS_MASK);	
}

SkButton::SkButton(const Glib::RefPtr<Gdk::Pixbuf>& img, int width, int height)
:	mState(0)
{	
	set_events(Gdk::BUTTON_PRESS_MASK);
	SetPixbuf(img, width, height);
}

SkButton::~SkButton() {}

void SkButton::SetPixbuf(const Glib::RefPtr<Gdk::Pixbuf>& img, int width, int height)
{
	assert(img->get_width() == width ||
		   img->get_height() == height);
	
	mPixbuf = img;
	mSize.width = width;
	mSize.height = height;
	set_size_request(width, height);
}

bool SkButton::on_expose_event			(GdkEventExpose* ev)
{
	if (ev && mPixbuf)
	{
		gdk_window_clear(ev->window);
		gdk_pixbuf_render_to_drawable(mPixbuf->gobj(), ev->window,
									  get_style()->get_black_gc()->gobj(),
									  mSize.width, mSize.height*mState,
									  0, 0, mSize.width, mSize.height,
									  GDK_RGB_DITHER_NONE, 0, 0);
	}
}

bool SkButton::on_button_press_event	(GdkEventButton* ev)
{
	mState = mState ? 0 : 1;
	queue_draw();
}
