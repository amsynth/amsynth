/*
 *  SkButton.h
 *  amsynth
 *  (c) 2006 Nick Dowell
 */

#ifndef _SkButton_h
#define _SkButton_h

#include <gtkmm.h>

class SkButton : public Gtk::Misc
{
public:
	~SkButton();
	
	SkButton();
	SkButton(const Glib::RefPtr<Gdk::Pixbuf>& img, int width, int height);
	
	void SetPixbuf(const Glib::RefPtr<Gdk::Pixbuf>& img, int width, int height);
	
protected:
	virtual bool on_expose_event			(GdkEventExpose*);
	virtual bool on_button_press_event		(GdkEventButton*);
	
private:
	Glib::RefPtr<Gdk::Pixbuf> mPixbuf;
	Gtk::Requisition mSize;
	int mState;
};

#endif
