
#include "bitmap_knob.h"

#include "../controls.h"

#include <math.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////

#define SENSITIVITY_STEP     40 // Pixels required to step up to next value
#define SENSITIVITY_NORMAL  300 // Pixels required to travel full range
#define SENSITIVITY_HIGH   1200 // Pixels required to travel full range

////////////////////////////////////////////////////////////////////////////////

typedef struct {

	GtkWidget *drawing_area;
	GtkWidget *tooltip_window;
	GtkWidget *tooltip_label;

	GtkAdjustment *adjustment;
	unsigned long parameter_index;
	
	GdkPixbuf *pixbuf;
	GdkPixbuf *background;
	guint current_frame;
	guint frame_width;
	guint frame_height;
	guint frame_count;
	guint sensitivity;
	
	gboolean is_dragging;
	gdouble origin_y;
	gdouble origin_val;
	
} bitmap_knob;

static const gchar *bitmap_knob_key = "bitmap_knob";

////////////////////////////////////////////////////////////////////////////////

static gboolean bitmap_knob_expose			( GtkWidget *wigdet, GdkEventExpose *event );
static gboolean bitmap_knob_button_press	( GtkWidget *wigdet, GdkEventButton *event );
static gboolean bitmap_knob_button_release	( GtkWidget *wigdet, GdkEventButton *event );
static gboolean bitmap_knob_motion_notify	( GtkWidget *wigdet, GdkEventMotion *event );

static void		bitmap_knob_set_adjustment				( GtkWidget *widget, GtkAdjustment *adjustment );
static void		bitmap_knob_adjustment_changed			( GtkAdjustment *adjustment, gpointer data );
static void		bitmap_knob_adjustment_value_changed	( GtkAdjustment *adjustment, gpointer data );

////////////////////////////////////////////////////////////////////////////////

GtkWidget *
bitmap_knob_new( GtkAdjustment *adjustment,
                 GdkPixbuf *pixbuf,
                 guint frame_width,
                 guint frame_height,
                 guint frame_count )
{
	bitmap_knob *self = g_malloc0 (sizeof(bitmap_knob));

	self->drawing_area = gtk_drawing_area_new ();
	self->pixbuf		= g_object_ref (pixbuf);
	self->frame_width	= frame_width;
	self->frame_height	= frame_height;
	self->frame_count	= frame_count;

	g_object_set_data_full (G_OBJECT (self->drawing_area), bitmap_knob_key, self, (GtkDestroyNotify) g_free);
	g_assert (g_object_get_data (G_OBJECT (self->drawing_area), bitmap_knob_key));
	
	g_signal_connect (G_OBJECT (self->drawing_area), "expose-event", G_CALLBACK (bitmap_knob_expose), NULL);
	g_signal_connect (G_OBJECT (self->drawing_area), "button-press-event", G_CALLBACK (bitmap_knob_button_press), NULL);
	g_signal_connect (G_OBJECT (self->drawing_area), "button-release-event", G_CALLBACK (bitmap_knob_button_release), NULL);
	g_signal_connect (G_OBJECT (self->drawing_area), "motion-notify-event", G_CALLBACK (bitmap_knob_motion_notify), NULL);
	
	gtk_widget_set_usize (self->drawing_area, frame_width, frame_height);
	
	// set up event mask
	gint event_mask = gtk_widget_get_events (self->drawing_area);
	event_mask |= GDK_BUTTON_PRESS_MASK;
	event_mask |= GDK_BUTTON_RELEASE_MASK;
	event_mask |= GDK_BUTTON1_MOTION_MASK;
	gtk_widget_set_events (self->drawing_area, event_mask);
	
	bitmap_knob_set_adjustment (self->drawing_area, adjustment);

	/* tooltip */

	self->tooltip_window = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_window_set_type_hint (GTK_WINDOW (self->tooltip_window), GDK_WINDOW_TYPE_HINT_TOOLTIP);

	static const guint tooltip_padding = 5;
	GtkWidget *alignment = gtk_alignment_new (0.5, 0.5, 1.0, 1.0);
	gtk_alignment_set_padding (GTK_ALIGNMENT (alignment),
		tooltip_padding, tooltip_padding, tooltip_padding, tooltip_padding);
	gtk_container_add (GTK_CONTAINER (self->tooltip_window), alignment);
	gtk_widget_show (alignment);

	self->tooltip_label = gtk_label_new ("");
	gtk_container_add (GTK_CONTAINER (alignment), self->tooltip_label);
	gtk_widget_show (self->tooltip_label);
	
	return self->drawing_area;
}

void bitmap_knob_set_bg (GtkWidget *widget, GdkPixbuf *pixbuf)
{
	bitmap_knob *self = g_object_get_data (G_OBJECT (widget), bitmap_knob_key);

	if (self->background) {
		g_object_unref (G_OBJECT (self->background));
	}

	self->background = pixbuf ? g_object_ref (G_OBJECT (pixbuf)) : NULL;

	gtk_widget_queue_draw (widget);
}

void bitmap_knob_set_parameter_index (GtkWidget *widget, unsigned long parameter_index)
{
	bitmap_knob *self = g_object_get_data (G_OBJECT (widget), bitmap_knob_key);
	self->parameter_index = parameter_index;
}

////////////////////////////////////////////////////////////////////////////////

static void tooltip_update (bitmap_knob *self)
{
	gdouble value = gtk_adjustment_get_value (self->adjustment);
	const char *name = parameter_name_from_index (self->parameter_index);
	char display[32] = "";
	parameter_get_display (self->parameter_index, value, display, sizeof(display));
	char text[64] = "";
	snprintf (text, sizeof(text), "%s: %s", name, display);
	gtk_label_set_text (GTK_LABEL (self->tooltip_label), text);
}

static void tooltip_show (bitmap_knob *self)
{
	gint x = 100, y = 100;
	GdkScreen *screen = NULL;
	GdkDisplay *display = gtk_widget_get_display (self->drawing_area);
	gdk_display_get_pointer (display, &screen, &x, &y, NULL);
	guint cursor_size = gdk_display_get_default_cursor_size (display);
	x += cursor_size / 2;
	y += cursor_size / 2;
	
	gtk_window_move (GTK_WINDOW (self->tooltip_window), x, y);
	gtk_widget_show (self->tooltip_window);
	return;
}

////////////////////////////////////////////////////////////////////////////////

static gboolean
bitmap_knob_expose( GtkWidget *widget, GdkEventExpose *event )
{
	bitmap_knob *self = g_object_get_data (G_OBJECT (widget), bitmap_knob_key);
	
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
		bitmap_knob *self = g_object_get_data (G_OBJECT (widget), bitmap_knob_key);
		gdouble lower = gtk_adjustment_get_lower (self->adjustment);
		gdouble upper = gtk_adjustment_get_upper (self->adjustment);
		gdouble step  = gtk_adjustment_get_step_increment (self->adjustment);
		if (step == 0.0) {
			self->sensitivity = (event->state & GDK_SHIFT_MASK) ? SENSITIVITY_HIGH : SENSITIVITY_NORMAL;
		} else {
			self->sensitivity = SENSITIVITY_STEP * (guint)((upper - lower) / step);
		}
		self->origin_val = gtk_adjustment_get_value (self->adjustment);
		self->origin_y = event->y;
		self->is_dragging = TRUE;
		tooltip_update (self);
		tooltip_show (self);
		return TRUE;
	}
	return FALSE;
}

gboolean
bitmap_knob_button_release ( GtkWidget *widget, GdkEventButton *event )
{
	if (event->button == 1) {
		bitmap_knob *self = g_object_get_data (G_OBJECT (widget), bitmap_knob_key);
		gtk_widget_hide (self->tooltip_window);
		self->is_dragging = FALSE;
		return TRUE;
	}
	return FALSE;
}

gboolean
bitmap_knob_motion_notify ( GtkWidget *widget, GdkEventMotion *event )
{
	if (event->state & GDK_BUTTON1_MASK)
	{
		bitmap_knob *self = g_object_get_data (G_OBJECT (widget), bitmap_knob_key);
		gdouble lower = gtk_adjustment_get_lower (self->adjustment);
		gdouble upper = gtk_adjustment_get_upper (self->adjustment);
		gdouble step  = gtk_adjustment_get_step_increment (self->adjustment);
		gdouble range = upper - lower;
		gdouble offset = (self->origin_y - event->y) * range / self->sensitivity;
		gdouble newval = self->origin_val + ((step == 0.0) ? offset : step * floor ((offset / step) + 0.5));
		gtk_adjustment_set_value (self->adjustment, CLAMP (newval, lower, upper));
		tooltip_update (self);
		return TRUE;
	}
	return FALSE;
}

void
bitmap_knob_update ( GtkWidget *widget )
{
	bitmap_knob *self = g_object_get_data (G_OBJECT (widget), bitmap_knob_key);
	
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
	bitmap_knob *self = g_object_get_data (G_OBJECT (widget), bitmap_knob_key);

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

