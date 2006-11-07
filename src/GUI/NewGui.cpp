/*
 *  GuiContainer.cpp
 *  amsynth
 *  (c) 2006 Nick Dowell
 *
 */

#include "NewGui.h"

#include "knob_med.h"
#include "../Preset.h"

#include <libxml/parser.h>
#include <libxml/tree.h>

typedef Glib::RefPtr<Gdk::Pixbuf> PixbufPtr;

////////////////////////////////////////////////////////////////////////////////

class SkWidget : public Gtk::Misc
{
public:
	
	SkWidget()
	{
		set_events (Gdk::BUTTON_PRESS_MASK | 
					Gdk::BUTTON_RELEASE_MASK | 
					Gdk::POINTER_MOTION_MASK | 
					Gdk::KEY_PRESS_MASK);
	}
	
	void set_background(Glib::RefPtr<Gdk::Pixbuf> pixbuf) { mBackground = pixbuf; }
	
	void draw_pixmap(PixbufPtr pixbuf,
					 int src_x, int src_y,
					 int dest_x, int dest_y,
					 int width, int height)
	{
			gdk_pixbuf_render_to_drawable(mBackground->gobj(), get_window()->gobj(),
										  get_style()->get_black_gc()->gobj(),
										  0, 0, 0, 0, get_width(), get_height(),
										  GDK_RGB_DITHER_NONE, 0, 0);
			
			gdk_pixbuf_render_to_drawable_alpha(pixbuf->gobj(), get_window()->gobj(),
												src_x, src_y, dest_x, dest_y, width, height,
												GDK_PIXBUF_ALPHA_FULL, 0,
												GDK_RGB_DITHER_NONE, 0, 0);
	}
	
	virtual void on_mouse_in() {}
	virtual void on_mouse_out() {}
	virtual void on_mouse_move(GdkEventMotion * ev) = 0;
	virtual bool on_mouse_button_press(GdkEventButton * ev) = 0;
	virtual bool on_key_press_event(GdkEventKey* event)
	{
		printf("key %x pressed\n", event->keyval);
		return true;
	}
	
	
	virtual bool on_button_press_event(GdkEventButton * ev)
	{
		grab_focus();
		on_mouse_button_press(ev);
		return true;
	}
	
	virtual bool on_motion_notify_event(GdkEventMotion * ev)
	{
		if (SkWidget::s_lastMouseOver != this)
		{
			if (SkWidget::s_lastMouseOver) 
			{
				SkWidget::s_lastMouseOver->on_mouse_out();
			}
			SkWidget::s_lastMouseOver = this;
			on_mouse_in();
		}
		on_mouse_move(ev);
		return true;
	}
	
	static SkWidget * s_lastMouseOver;
	
private:;
	
	PixbufPtr mBackground;

};

SkWidget * SkWidget::s_lastMouseOver = NULL;

////////////////////////////////////////////////////////////////////////////////

class ISkControlListener
{
public:
	virtual void on_control_value_changed(int paramId, float value) = 0;
};

////////////////////////////////////////////////////////////////////////////////

class SkControl : public SkWidget
{
public:
	SkControl() : mParamValue(0.0), mParamId(0), mListener(NULL) {}
	
	void set_param_id(int id) { mParamId = id; }
	void set_param_value(float value)
	{
		if (mParamValue != value) {
			mParamValue = value;
			on_value_changed();
		}
	}
	
	void set_update_listener(ISkControlListener * cl) { mListener = cl; }
	
protected:
	void set_value(float value)
	{
		if (mParamValue != value) {
			mParamValue  = value;
			on_value_changed();
			update_listener();
		}
	}
	void update_listener() { 
		if (mListener) {
			mListener->on_control_value_changed(mParamId, mParamValue);
		}
	}
	
	virtual void on_value_changed() = 0;
	
	float mParamValue;
	int   mParamId;

private:
	ISkControlListener * mListener;
};

////////////////////////////////////////////////////////////////////////////////

class NewKnob : public SkControl
{
public:
	
	NewKnob()
	:	mNumFrames(49)
	,	mCurFrame(0)
	{
		Glib::RefPtr<Gdk::PixbufLoader> ldr = Gdk::PixbufLoader::create();
		ldr->write(knob_med_png, sizeof(knob_med_png)); ldr->close();
		mImgData = ldr->get_pixbuf();
		
		set_size_request(48,48);
	}
	
	virtual bool on_mouse_button_press(GdkEventButton * ev)
	{
		mDragOrigin.x = ev->x;
		mDragOrigin.y = ev->x;
		mDragOrigin.val = mParamValue;
		add_modal_grab();
		return true;
	}
	
	virtual void on_mouse_move(GdkEventMotion * ev)
	{
		if (has_grab())
		{
			const float kPixelScale = 1.f / 256.f;
			int delta = (ev->x - mDragOrigin.x) + (mDragOrigin.y - ev->y);
			SetValue(mDragOrigin.val + (kPixelScale * (float)delta));
		}
	}
	
	virtual void on_mouse_in() {}
	virtual void on_mouse_out() {}
	
	virtual bool on_button_release_event(GdkEventButton * ev)
	{
		if (has_grab()) remove_modal_grab();
		return TRUE;
	}

	void SetValue(float value)
	{
		if (value < 0.0) value = 0.0; 
		if (value > 1.0) value = 1.0;
		set_value(value);
	}
	
	virtual void on_value_changed()
	{
		int frame = mParamValue * (mNumFrames - 1);
		if (mCurFrame != frame) {
			mCurFrame  = frame;
			queue_draw();
		}
	}
	
	virtual bool on_expose_event(GdkEventExpose *)
	{
		draw_pixmap(mImgData, 0, get_height() * mCurFrame, 0, 0, get_width(), get_height());
	}

private:
	struct { int x, y; float val; } mDragOrigin;
	struct { float x, y; } mResolution;

	PixbufPtr mImgData;
	int mNumFrames;
	int mCurFrame;
};

////////////////////////////////////////////////////////////////////////////////

class GuiContainer : public Gtk::Layout, public ISkControlListener, public UpdateListener
{
public:
	
	GuiContainer(Preset *);
	~GuiContainer();
	
	void load_layout();
	
	void put(SkWidget & child_widget, int x, int y);
	void move(SkWidget & child_widget, int x, int y);
	
	// event handlers
	virtual bool on_expose_event(GdkEventExpose *);
	virtual bool on_motion_notify_event(GdkEventMotion *);
	
	// events controls
	virtual void on_control_value_changed(int paramId, float value);
	// from synth core
	virtual void UpdateParameter(Param param, float value);
	
private: ;
	
	PixbufPtr mBackground;
	
	Parameter * mParams[kControls_End];
	SkControl * mParamListeners[kControls_End];
	
	Gtk::Label * mLabel;
};

////////////////////

GuiContainer::GuiContainer(Preset * p)
{
	for (int i=0; i<kControls_End; i++) { 
		mParams[i] = &(p->getParameter(i));
		mParamListeners[i] = NULL;
	}
	
	set_events (Gdk::POINTER_MOTION_MASK);
	
	load_layout();
	
	p->AddListenerToAll(this);
	
	mLabel = manage(new Gtk::Label("amSynth"));
	Gtk::Layout::put(* mLabel, 300, 300);
	mLabel->show();
}

void GuiContainer::load_layout()
{
	xmlDoc * doc = xmlReadFile("/Users/nick/dev/amsynthe/src/GUI/layout.xml", NULL, 0);
	xmlNode * root = xmlDocGetRootElement(doc);
	xmlChar * prop = NULL;
	
	prop = xmlGetProp(root, (xmlChar *)"img");
	
	mBackground = Gdk::Pixbuf::create_from_file((char *)prop);
	set_size_request(mBackground->get_width(),
					 mBackground->get_height());
	
	xmlFree(prop);
	
	xmlNode * node = root->children;
	while(node)
	{
		if ((node->type == XML_ELEMENT_NODE) &&
			(strcmp((char *)node->name, "Knob") == 0))
		{
			int x, y, paramId;
			
			prop = xmlGetProp(node, (xmlChar *)"pos_x");
			x = atoi((char *)prop);
			xmlFree(prop);
			
			prop = xmlGetProp(node, (xmlChar *)"pos_y");
			y = atoi((char *)prop);
			xmlFree(prop);
			
			prop = xmlGetProp(node, (xmlChar *)"paramId");
			paramId = atoi((char *)prop);
			xmlFree(prop);
			
			SkControl * ctl = manage(new NewKnob);
			ctl->set_update_listener(this);
			ctl->set_param_id(paramId);
			mParamListeners[paramId] = ctl;
			put(*ctl, x, y);
		}
		node = node->next;
	}
	
	xmlFreeDoc(doc);
	xmlCleanupParser();
}

GuiContainer::~GuiContainer()
{
}

void GuiContainer::put(SkWidget & child, int x, int y)
{
	child.set_background(Gdk::Pixbuf::create_subpixbuf(mBackground, x, y, child.get_width(), child.get_height()));
	Gtk::Layout::put(child, x, y);
}

void GuiContainer::move(SkWidget & child, int x, int y)
{
	child.set_background(Gdk::Pixbuf::create_subpixbuf(mBackground, x, y, child.get_width(), child.get_height()));
	Gtk::Layout::move(child, x, y);
}

// event handlers
bool GuiContainer::on_expose_event(GdkEventExpose*)
{
	if (mBackground)
	{		
		gdk_pixbuf_render_to_drawable(mBackground->gobj(), get_bin_window()->gobj(),
									  get_style()->get_black_gc()->gobj(),
									  0, 0, 0, 0, get_width(), get_height(),
									  GDK_RGB_DITHER_NONE, 0, 0);
	}
	
	return true;
}

bool GuiContainer::on_motion_notify_event(GdkEventMotion * ev)
{
	if (SkWidget::s_lastMouseOver) {
		SkWidget::s_lastMouseOver->on_mouse_out();
		SkWidget::s_lastMouseOver = NULL;
	}
	return TRUE;
}

void GuiContainer::on_control_value_changed(int paramId, float value)
{
	Parameter * param = mParams[paramId];
	param->SetNormalisedValue(value);
//	char str[32]; sprintf(str, "%s : %f", param->getName().c_str(), param->getValue());
//	mLabel->set_text(str);
}

void GuiContainer::UpdateParameter(Param paramId, float value)
{
	if (mParamListeners[paramId]) {
		Parameter * param = mParams[paramId];
		float value = param->GetNormalisedValue();
		mParamListeners[paramId]->set_param_value(value);
	}
}

Gtk::Widget * CreateNewGui(Preset * p) { return new GuiContainer(p); }
