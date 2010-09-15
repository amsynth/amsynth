
#include "bitmap_popup.h"

////////////////////////////////////////////////////////////////////////////////

#define BITMAP_POPUP(obj)				(G_TYPE_CHECK_INSTANCE_CAST	((obj), bitmap_popup_get_type(), BitmapPopup))
#define BITMAP_POPUP_CLASS(obj)			(G_TYPE_CHECK_CLASS_CAST	((obj), BITMAP_POPUP,			BitmapPopupClass))
#define GTK_IS_BITMAP_POPUP(obj)		(G_TYPE_CHECK_INSTANCE_TYPE	((obj), bitmap_popup_get_type()))
#define GTK_IS_BITMAP_POPUP_CLASS(obj)	(G_TYPE_CHECK_CLASS_TYPE	((obj), bitmap_popup_get_type()))
#define BITMAP_POPUP_GET_CLASS			(G_TYPE_INSTANCE_GET_CLASS	((obj), bitmap_popup_get_type(),	BitmapPopupClass))

typedef struct _BitmapPopup		BitmapPopup;
typedef struct _BitmapPopupClass	BitmapPopupClass;

struct _BitmapPopup
{
	GtkDrawingArea parent;

	GtkAdjustment *adjustment;
	
	GdkPixbuf *pixbuf;
	GdkPixbuf *background;
	guint current_frame;
	guint frame_width;
	guint frame_height;
	guint frame_count;
	
	GtkWidget *menu;
};

struct _BitmapPopupClass
{
	GtkDrawingAreaClass parent_class;
};

////////////////////////////////////////////////////////////////////////////////

G_DEFINE_TYPE( BitmapPopup, bitmap_popup, GTK_TYPE_DRAWING_AREA );

static gboolean bitmap_popup_expose			( GtkWidget *wigdet, GdkEventExpose *event );
static gboolean bitmap_popup_button_press	( GtkWidget *wigdet, GdkEventButton *event );

static void		bitmap_popup_set_adjustment				( GtkWidget *widget, GtkAdjustment *adjustment );
static void		bitmap_popup_adjustment_changed			( GtkAdjustment *adjustment, gpointer data );
static void		bitmap_popup_adjustment_value_changed	( GtkAdjustment *adjustment, gpointer data );
static void		bitmap_popup_menuitem_activated			( GtkWidget *menu_item, gpointer data );

////////////////////////////////////////////////////////////////////////////////

static void
bitmap_popup_class_init( BitmapPopupClass *aclass )
{
	GtkWidgetClass *widget_class		= GTK_WIDGET_CLASS( aclass );
	widget_class->expose_event			= bitmap_popup_expose;
	widget_class->button_press_event	= bitmap_popup_button_press;
}

static void
bitmap_popup_init( BitmapPopup *self )
{
	self->adjustment	= NULL;
	self->pixbuf		= NULL;
	self->background	= NULL;
	self->current_frame	= 0;
	self->frame_width	= 1;
	self->frame_height	= 1;
	self->frame_count	= 1;
	self->menu			= NULL;
}

////////////////////////////////////////////////////////////////////////////////

GtkWidget *
bitmap_popup_new( GtkAdjustment *adjustment,
                 GdkPixbuf *pixbuf,
                 guint frame_width,
                 guint frame_height,
                 guint frame_count )
{
	GtkWidget *widget = g_object_new (bitmap_popup_get_type(), NULL);
	
	BitmapPopup *self = BITMAP_POPUP (widget);
	
	self->pixbuf		= g_object_ref (pixbuf);
	self->frame_width	= frame_width;
	self->frame_height	= frame_height;
	self->frame_count	= frame_count;
	
	gtk_widget_set_usize (widget, frame_width, frame_height);
	
	// set up event mask
	gint event_mask = gtk_widget_get_events (widget);
	event_mask |= GDK_BUTTON_PRESS_MASK;
	gtk_widget_set_events (widget, event_mask);
	
	bitmap_popup_set_adjustment (widget, adjustment);
	
	return widget;
}

void bitmap_popup_set_bg (GtkWidget *widget, GdkPixbuf *pixbuf)
{
	BitmapPopup *self = BITMAP_POPUP (widget);

	if (self->background)
	{
		gtk_object_unref (GTK_OBJECT (self->background));
	}
	
	self->background = g_object_ref (G_OBJECT (pixbuf));
}

void bitmap_popup_set_strings (GtkWidget *widget, const gchar **strings)
{
	BitmapPopup *self = BITMAP_POPUP (widget);
	
	if (self->menu)
	{
		g_object_unref (G_OBJECT (self->menu));
	}

	self->menu = gtk_menu_new ();
	
	gtk_menu_attach_to_widget (GTK_MENU (self->menu), widget, NULL);
	
	guint i, max = gtk_adjustment_get_upper (self->adjustment);
	for (i = 0; i <= max; i++)
	{
		gchar *label = g_strstrip (g_strdup(strings[i]));
		GtkWidget *menu_item = gtk_menu_item_new_with_label (label);
		gtk_signal_connect (GTK_OBJECT (menu_item), "activate",
			(GtkSignalFunc) bitmap_popup_menuitem_activated,
			(gpointer) self );
		gtk_menu_append (self->menu, menu_item);
		g_object_unref (G_OBJECT (menu_item));
		g_free (label);
	}
	
	gtk_widget_show_all (self->menu);
}

////////////////////////////////////////////////////////////////////////////////

static gboolean
bitmap_popup_expose( GtkWidget *widget, GdkEventExpose *event )
{
	BitmapPopup *self = BITMAP_POPUP (widget);
	
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
bitmap_popup_button_press ( GtkWidget *widget, GdkEventButton *event )
{
	if (event->type == GDK_BUTTON_PRESS && event->button == 1)
	{
		BitmapPopup *self = BITMAP_POPUP (widget);
		gtk_menu_popup (GTK_MENU (self->menu), NULL, NULL, NULL, NULL, event->button, event->time);
		return TRUE;
	}
	return FALSE;
}

void
bitmap_popup_menuitem_activated (GtkWidget *menu_item, gpointer data)
{
	BitmapPopup *self = BITMAP_POPUP (data);
	
	GList *list = gtk_container_get_children (GTK_CONTAINER (self->menu));
	int i = g_list_index (list, menu_item);
	g_list_free (list);
	
	gtk_adjustment_set_value (self->adjustment, i);
}

void
bitmap_popup_update (GtkWidget *widget)
{
	BitmapPopup *self = BITMAP_POPUP (widget);
	
	gdouble value = gtk_adjustment_get_value (self->adjustment);
	gdouble lower = gtk_adjustment_get_lower (self->adjustment);
	gdouble upper = gtk_adjustment_get_upper (self->adjustment);
	guint	frame = self->frame_count * ((value - lower) / (upper - lower));

	self->current_frame = MIN (frame, (self->frame_count - 1));
	
	gtk_widget_queue_draw (widget);
}

void
bitmap_popup_adjustment_changed			( GtkAdjustment *adjustment, gpointer data )
{
	bitmap_popup_update (data);
}

void
bitmap_popup_adjustment_value_changed	( GtkAdjustment *adjustment, gpointer data )
{
	bitmap_popup_update (data);
}

void
bitmap_popup_set_adjustment( GtkWidget *widget, GtkAdjustment *adjustment )
{
	BitmapPopup *self = BITMAP_POPUP (widget);

	if (self->adjustment)
	{
		gtk_signal_disconnect_by_data (GTK_OBJECT (self->adjustment), (gpointer) self);
		gtk_object_unref (GTK_OBJECT (self->adjustment) );
	}
	
	self->adjustment = g_object_ref (GTK_OBJECT (adjustment) );

	gtk_signal_connect (GTK_OBJECT (adjustment), "changed",
		(GtkSignalFunc) bitmap_popup_adjustment_changed,
		(gpointer) widget );
		
	gtk_signal_connect (GTK_OBJECT (adjustment), "value_changed",
		(GtkSignalFunc) bitmap_popup_adjustment_value_changed,
		(gpointer) widget );
	
	bitmap_popup_adjustment_changed (adjustment, widget);
}

////////////////////////////////////////////////////////////////////////////////

