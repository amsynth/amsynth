
#include "bitmap_button.h"

////////////////////////////////////////////////////////////////////////////////

#define BITMAP_BUTTON(obj)				(G_TYPE_CHECK_INSTANCE_CAST	((obj), bitmap_button_get_type(),	BitmapButton))
#define BITMAP_BUTTON_CLASS(obj)		(G_TYPE_CHECK_CLASS_CAST	((obj), BITMAP_BUTTON,				BitmapButtonClass))
#define GTK_IS_BITMAP_BUTTON(obj)		(G_TYPE_CHECK_INSTANCE_TYPE	((obj), bitmap_button_get_type()))
#define GTK_IS_BITMAP_BUTTON_CLASS(obj)	(G_TYPE_CHECK_CLASS_TYPE	((obj), bitmap_button_get_type()))
#define BITMAP_BUTTON_GET_CLASS			(G_TYPE_INSTANCE_GET_CLASS	((obj), bitmap_button_get_type(),	BitmapButtonClass))

typedef struct _BitmapButton		BitmapButton;
typedef struct _BitmapButtonClass	BitmapButtonClass;

struct _BitmapButton
{
	GtkDrawingArea parent;

	GtkAdjustment *adjustment;
	
	GdkPixbuf *pixbuf;
	GdkPixbuf *background;
	guint current_frame;
	guint frame_width;
	guint frame_height;
	guint frame_count;
};

struct _BitmapButtonClass
{
	GtkDrawingAreaClass parent_class;
};

////////////////////////////////////////////////////////////////////////////////

G_DEFINE_TYPE( BitmapButton, bitmap_button, GTK_TYPE_DRAWING_AREA );

static gboolean bitmap_button_expose		( GtkWidget *wigdet, GdkEventExpose *event );
static gboolean bitmap_button_button_press	( GtkWidget *wigdet, GdkEventButton *event );

static void		bitmap_button_set_adjustment			( GtkWidget *widget, GtkAdjustment *adjustment );
static void		bitmap_button_adjustment_changed		( GtkAdjustment *adjustment, gpointer data );
static void		bitmap_button_adjustment_value_changed	( GtkAdjustment *adjustment, gpointer data );

////////////////////////////////////////////////////////////////////////////////

static void
bitmap_button_class_init( BitmapButtonClass *aclass )
{
	GtkWidgetClass *widget_class		= GTK_WIDGET_CLASS( aclass );
	widget_class->expose_event			= bitmap_button_expose;
	widget_class->button_press_event	= bitmap_button_button_press;
}

static void
bitmap_button_init( BitmapButton *self )
{
	self->adjustment	= NULL;
	self->pixbuf		= NULL;
	self->background	= NULL;
	self->current_frame	= 0;
	self->frame_width	= 1;
	self->frame_height	= 1;
	self->frame_count	= 1;
}

////////////////////////////////////////////////////////////////////////////////

GtkWidget *
bitmap_button_new( GtkAdjustment *adjustment,
                 GdkPixbuf *pixbuf,
                 guint frame_width,
                 guint frame_height,
                 guint frame_count )
{
	GtkWidget *widget = g_object_new (bitmap_button_get_type(), NULL);

	BitmapButton *self = BITMAP_BUTTON (widget);
	
	self->pixbuf		= g_object_ref (pixbuf);
	self->frame_width	= frame_width;
	self->frame_height	= frame_height;
	self->frame_count	= frame_count;
	
	gtk_widget_set_usize (widget, frame_width, frame_height);
	
	// set up event mask
	gint event_mask = gtk_widget_get_events (widget);
	event_mask |= GDK_BUTTON_PRESS_MASK;
	gtk_widget_set_events (widget, event_mask);
	
	bitmap_button_set_adjustment (widget, adjustment);
	
	return widget;
}

void bitmap_button_set_bg (GtkWidget *widget, GdkPixbuf *pixbuf)
{
	BitmapButton *self = BITMAP_BUTTON (widget);

	if (self->background)
	{
		gtk_object_unref (GTK_OBJECT (self->background));
	}
	
	self->background = g_object_ref (G_OBJECT (pixbuf));
}

////////////////////////////////////////////////////////////////////////////////

static gboolean
bitmap_button_expose( GtkWidget *widget, GdkEventExpose *event )
{
	BitmapButton *self = BITMAP_BUTTON (widget);
	
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
	
	gdk_draw_pixbuf (
		widget->window,
		NULL,	// gc
		self->pixbuf,
		0,	// src_x
		self->current_frame * self->frame_height,
		0,	// dest_x
		0,	// dest_y
		self->frame_width,
		self->frame_height,
		GDK_RGB_DITHER_NONE, 0, 0
	);
	
	return FALSE;
}

gboolean
bitmap_button_button_press ( GtkWidget *widget, GdkEventButton *event )
{
	if (event->type == GDK_BUTTON_PRESS && event->button == 1)
	{
		BitmapButton *self = BITMAP_BUTTON (widget);
		gdouble value = gtk_adjustment_get_value (self->adjustment);
		gdouble lower = gtk_adjustment_get_lower (self->adjustment);
		gdouble upper = gtk_adjustment_get_upper (self->adjustment);
		gdouble middl = (upper - lower) / 2.0;		
		gtk_adjustment_set_value (self->adjustment, (value < middl) ? 1.0 : 0.0);
		return TRUE;
	}
	return FALSE;
}

void
bitmap_button_update (GtkWidget *widget)
{
	BitmapButton *self = BITMAP_BUTTON (widget);
	
	gdouble value = gtk_adjustment_get_value (self->adjustment);
	gdouble lower = gtk_adjustment_get_lower (self->adjustment);
	gdouble upper = gtk_adjustment_get_upper (self->adjustment);
	guint	frame = self->frame_count * ((value - lower) / (upper - lower));

	self->current_frame = MIN (frame, (self->frame_count - 1));
	
	gtk_widget_queue_draw (widget);
}

void
bitmap_button_adjustment_changed			( GtkAdjustment *adjustment, gpointer data )
{
	bitmap_button_update (data);
}

void
bitmap_button_adjustment_value_changed	( GtkAdjustment *adjustment, gpointer data )
{
	bitmap_button_update (data);
}

void
bitmap_button_set_adjustment( GtkWidget *widget, GtkAdjustment *adjustment )
{
	BitmapButton *self = BITMAP_BUTTON (widget);

	if (self->adjustment)
	{
		gtk_signal_disconnect_by_data (GTK_OBJECT (self->adjustment), (gpointer) self);
		gtk_object_unref (GTK_OBJECT (self->adjustment) );
	}
	
	self->adjustment = g_object_ref (GTK_OBJECT (adjustment) );

	gtk_signal_connect (GTK_OBJECT (adjustment), "changed",
		(GtkSignalFunc) bitmap_button_adjustment_changed,
		(gpointer) widget );
		
	gtk_signal_connect (GTK_OBJECT (adjustment), "value_changed",
		(GtkSignalFunc) bitmap_button_adjustment_value_changed,
		(gpointer) widget );
	
	bitmap_button_adjustment_changed (adjustment, widget);
}

////////////////////////////////////////////////////////////////////////////////
