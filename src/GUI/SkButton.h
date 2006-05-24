/*
 *  SkButton.h
 *  amsynth
 *  (c) 2006 Nick Dowell
 */

#ifndef _SkButton_h
#define _SkButton_h

#include <gtkmm.h>
#include "../Parameter.h"

class SkButton : public Gtk::Misc, public UpdateListener
{
public:
	~SkButton();
	
	SkButton();
	SkButton(const Glib::RefPtr<Gdk::Pixbuf>& img, int width, int height);
	
	void SetPixbuf(const Glib::RefPtr<Gdk::Pixbuf>& img, int width, int height);
	
	void BindToParameter(Parameter*);
	virtual void UpdateParameter(Param, float);
	
protected:
	virtual bool on_expose_event			(GdkEventExpose*);
	virtual bool on_button_press_event		(GdkEventButton*);
	
private:
	Glib::RefPtr<Gdk::Pixbuf> mPixbuf;
	Gtk::Requisition mSize;
	Parameter* mParam;
	int mState;
};

#endif
