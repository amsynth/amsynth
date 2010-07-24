
#include "bitmap_knob.h"

#include <math.h>

////////////////////////////////////////////////////////////////////////////////

#define BITMAP_KNOB(obj)				(G_TYPE_CHECK_INSTANCE_CAST	((obj), bitmap_knob_get_type(), BitmapKnob))
#define BITMAP_KNOB_CLASS(obj)			(G_TYPE_CHECK_CLASS_CAST	((obj), BITMAP_KNOB,			BitmapKnobClass))
#define GTK_IS_BITMAP_KNOB(obj)			(G_TYPE_CHECK_INSTANCE_TYPE	((obj), bitmap_knob_get_type()))
#define GTK_IS_BITMAP_KNOB_CLASS(obj)	(G_TYPE_CHECK_CLASS_TYPE	((obj), bitmap_knob_get_type()))
#define BITMAP_KNOB_GET_CLASS			(G_TYPE_INSTANCE_GET_CLASS	((obj), bitmap_knob_get_type(),	BitmapKnobClass))

typedef struct _BitmapKnob		BitmapKnob;
typedef struct _BitmapKnobClass	BitmapKnobClass;

struct _BitmapKnob
{
	GtkDrawingArea parent;

	GtkAdjustment *adjustment;
	
	GdkPixbuf *pixbuf;
	GdkPixbuf *background;
	guint current_frame;
	guint frame_width;
	guint frame_height;
	guint frame_count;
	
	gdouble origin_y;
	gdouble origin_val;
};

struct _BitmapKnobClass
{
	GtkDrawingAreaClass parent_class;
};

////////////////////////////////////////////////////////////////////////////////

G_DEFINE_TYPE( BitmapKnob, bitmap_knob, GTK_TYPE_DRAWING_AREA );

static gboolean bitmap_knob_expose			( GtkWidget *wigdet, GdkEventExpose *event );
static gboolean bitmap_knob_button_press	( GtkWidget *wigdet, GdkEventButton *event );
static gboolean bitmap_knob_motion_notify	( GtkWidget *wigdet, GdkEventMotion *event );

static void		bitmap_knob_set_adjustment				( GtkWidget *widget, GtkAdjustment *adjustment );
static void		bitmap_knob_adjustment_changed			( GtkAdjustment *adjustment, gpointer data );
static void		bitmap_knob_adjustment_value_changed	( GtkAdjustment *adjustment, gpointer data );

////////////////////////////////////////////////////////////////////////////////

static void
bitmap_knob_class_init( BitmapKnobClass *aclass )
{
	GtkWidgetClass *widget_class		= GTK_WIDGET_CLASS( aclass );
	widget_class->expose_event			= bitmap_knob_expose;
	widget_class->button_press_event	= bitmap_knob_button_press;
	widget_class->motion_notify_event	= bitmap_knob_motion_notify;
}

static void
bitmap_knob_init( BitmapKnob *self )
{
	self->adjustment	= NULL;
	self->pixbuf		= NULL;
	self->background	= NULL;
	self->current_frame	= 0;
	self->frame_width	= 1;
	self->frame_height	= 1;
	self->frame_count	= 1;
	self->origin_val	= 0;
	self->origin_y		= 0;
}

////////////////////////////////////////////////////////////////////////////////

GtkWidget *
bitmap_knob_new( GtkAdjustment *adjustment,
                 GdkPixbuf *pixbuf,
                 guint frame_width,
                 guint frame_height,
                 guint frame_count )
{
	GtkWidget *widget = g_object_new (bitmap_knob_get_type(), NULL);
	
	BitmapKnob *self = BITMAP_KNOB (widget);
	
	self->pixbuf		= g_object_ref (pixbuf);
	self->frame_width	= frame_width;
	self->frame_height	= frame_height;
	self->frame_count	= frame_count;
	
	gtk_widget_set_usize (widget, frame_width, frame_height);
	
	// set up event mask
	gint event_mask = gtk_widget_get_events (widget);
	event_mask |= GDK_BUTTON_PRESS_MASK;
	event_mask |= GDK_BUTTON1_MOTION_MASK;
	gtk_widget_set_events (widget, event_mask);
	
	bitmap_knob_set_adjustment (widget, adjustment);
	
	return widget;
}

void bitmap_knob_set_bg (GtkWidget *widget, GdkPixbuf *pixbuf)
{
	BitmapKnob *self = BITMAP_KNOB (widget);

	if (self->background)
	{
		gtk_object_unref (GTK_OBJECT (self->background));
	}
	
	self->background = g_object_ref (G_OBJECT (pixbuf));
}

////////////////////////////////////////////////////////////////////////////////

static gboolean
bitmap_knob_expose( GtkWidget *widget, GdkEventExpose *event )
{
	BitmapKnob *self = BITMAP_KNOB (widget);
	
	if (self->background) {
		gdk_draw_pixbuf (
			widget->window,
			NULL,	// gc
			self->background,
			0,	// src_x
			0,	// src_y
			0,	// dest_x
			0,	// dest_y
			gdk_pixbuf_get_width (self->background),
			gdk_pixbuf_get_height (self->background),
			GDK_RGB_DITHER_NONE, 0, 0
		);	
	}
	
	guint src_x = 0, src_y = 0;
	
	if (gdk_pixbuf_get_height (self->pixbuf) == self->frame_height)
		src_x = self->current_frame * self->frame_width;
	else
		src_y = self->current_frame * self->frame_height;
	
	gdk_draw_pixbuf (
		widget->window,
		NULL,	// gc
		self->pixbuf,
		src_x,
		src_y,
		0,	// dest_x
		0,	// dest_y
		self->frame_width,
		self->frame_height,
		GDK_RGB_DITHER_NONE, 0, 0
	);
	
	return FALSE;
}

gboolean
bitmap_knob_button_press ( GtkWidget *widget, GdkEventButton *event )
{
	if (event->type == GDK_BUTTON_PRESS && event->button == 1)
	{
		BitmapKnob *self = BITMAP_KNOB( widget );
		self->origin_val = gtk_adjustment_get_value (self->adjustment);
		self->origin_y = event->y;
		return TRUE;
	}
	return FALSE;
}

gboolean
bitmap_knob_motion_notify ( GtkWidget *widget, GdkEventMotion *event )
{
	if (event->state & GDK_BUTTON1_MASK)
	{
		BitmapKnob *self = BITMAP_KNOB( widget );
		gdouble lower = gtk_adjustment_get_lower (self->adjustment);
		gdouble upper = gtk_adjustment_get_upper (self->adjustment);
		gdouble step  = gtk_adjustment_get_step_increment (self->adjustment);
		gdouble range = upper - lower;
		gdouble offset = (self->origin_y - event->y) * range / 400;
		gdouble newval = self->origin_val + ((step == 0.0) ? offset : step * floor ((offset / step) + 0.5));
		gtk_adjustment_set_value (self->adjustment, CLAMP (newval, lower, upper));
		return TRUE;
	}
	return FALSE;
}

void
bitmap_knob_update ( GtkWidget *widget )
{
	BitmapKnob *self = BITMAP_KNOB (widget);
	
	gdouble value = gtk_adjustment_get_value (self->adjustment);
	gdouble lower = gtk_adjustment_get_lower (self->adjustment);
	gdouble upper = gtk_adjustment_get_upper (self->adjustment);
	guint	frame = self->frame_count * ((value - lower) / (upper - lower));

	self->current_frame = MIN (frame, (self->frame_count - 1));
	
	gtk_widget_queue_draw (widget);
}

void
bitmap_knob_adjustment_changed			( GtkAdjustment *adjustment, gpointer data )
{
	bitmap_knob_update (data);
}

void
bitmap_knob_adjustment_value_changed	( GtkAdjustment *adjustment, gpointer data )
{
	bitmap_knob_update (data);
}

void
bitmap_knob_set_adjustment( GtkWidget *widget, GtkAdjustment *adjustment )
{
	BitmapKnob *self = BITMAP_KNOB (widget);

	if (self->adjustment)
	{
		gtk_signal_disconnect_by_data (GTK_OBJECT (self->adjustment), (gpointer) self);
		gtk_object_unref (GTK_OBJECT (self->adjustment) );
	}
	
	self->adjustment = g_object_ref (GTK_OBJECT (adjustment) );

	gtk_signal_connect (GTK_OBJECT (adjustment), "changed",
		(GtkSignalFunc) bitmap_knob_adjustment_changed,
		(gpointer) widget );
		
	gtk_signal_connect (GTK_OBJECT (adjustment), "value_changed",
		(GtkSignalFunc) bitmap_knob_adjustment_value_changed,
		(gpointer) widget );
	
	bitmap_knob_adjustment_changed (adjustment, widget);
}

////////////////////////////////////////////////////////////////////////////////

