/*
 *  bitmap_button.c
 *
 *  Copyright (c) 2001-2019 Nick Dowell
 *
 *  This file is part of amsynth.
 *
 *  amsynth is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  amsynth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with amsynth.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "bitmap_button.h"

////////////////////////////////////////////////////////////////////////////////

typedef struct {

	GtkWidget *drawing_area;
	GtkAdjustment *adjustment;
	GdkPixbuf *pixbuf;
	GdkPixbuf *background;
	gint current_frame;
	gint frame_width;
	gint frame_height;
	gint frame_count;
	gint scaling_factor

} bitmap_button;

static const gchar *bitmap_button_key = "bitmap_button";

////////////////////////////////////////////////////////////////////////////////

static gboolean bitmap_button_expose		( GtkWidget *widget, GdkEventExpose *event );
static gboolean bitmap_button_button_press	( GtkWidget *widget, GdkEventButton *event );

static void		bitmap_button_set_adjustment			( GtkWidget *widget, GtkAdjustment *adjustment );
static void		bitmap_button_adjustment_changed		( GtkAdjustment *adjustment, gpointer data );
static void		bitmap_button_adjustment_value_changed	( GtkAdjustment *adjustment, gpointer data );

////////////////////////////////////////////////////////////////////////////////

GtkWidget *
bitmap_button_new( GtkAdjustment *adjustment,
                 GdkPixbuf *pixbuf,
                 gint frame_width,
                 gint frame_height,
                 gint frame_count,
                 gint scaling_factor)
{
	bitmap_button *self = g_malloc0 (sizeof(bitmap_button));

	self->drawing_area = gtk_drawing_area_new ();
	self->pixbuf		= g_object_ref (pixbuf);
	self->frame_width	= frame_width;
	self->frame_height	= frame_height;
	self->frame_count	= frame_count;
	self->scaling_factor = scaling_factor;

	g_object_set_data_full (G_OBJECT (self->drawing_area), bitmap_button_key, self, (GDestroyNotify) g_free);
	g_assert (g_object_get_data (G_OBJECT (self->drawing_area), bitmap_button_key));

	g_signal_connect (G_OBJECT (self->drawing_area), "expose-event", G_CALLBACK (bitmap_button_expose), NULL);

	g_signal_connect (G_OBJECT (self->drawing_area), "button-press-event", G_CALLBACK (bitmap_button_button_press), NULL);

	gtk_widget_set_size_request (self->drawing_area, frame_width * scaling_factor, frame_height * scaling_factor);
	
	// set up event mask
	gint event_mask = gtk_widget_get_events (self->drawing_area);
	event_mask |= GDK_BUTTON_PRESS_MASK;
	gtk_widget_set_events (self->drawing_area, event_mask);
	
	bitmap_button_set_adjustment (self->drawing_area, adjustment);
	
	return self->drawing_area;
}

void bitmap_button_set_bg (GtkWidget *widget, GdkPixbuf *pixbuf)
{
	bitmap_button *self = g_object_get_data (G_OBJECT (widget), bitmap_button_key);

	if (self->background) {
		g_object_unref (G_OBJECT (self->background));
	}

	self->background = pixbuf ? GDK_PIXBUF (g_object_ref (G_OBJECT (pixbuf))) : NULL;

	gtk_widget_queue_draw (widget);
}

////////////////////////////////////////////////////////////////////////////////

gboolean
bitmap_button_expose ( GtkWidget *widget, GdkEventExpose *event )
{
	bitmap_button *self = g_object_get_data (G_OBJECT (widget), bitmap_button_key); g_assert (self);

	cairo_t *cr = gdk_cairo_create (event->window);

	cairo_scale (cr, self->scaling_factor, self->scaling_factor);

	if (self->background) {
		gdk_cairo_set_source_pixbuf (cr, self->background, 0, 0);
		// CAIRO_EXTEND_NONE results in a ugly border when upscaling
		cairo_pattern_t *pattern = cairo_get_source (cr);
		cairo_pattern_set_extend (pattern, CAIRO_EXTEND_PAD);
		cairo_paint (cr);
	}

	gdk_cairo_set_source_pixbuf (cr, self->pixbuf, 0, -self->current_frame * self->frame_height);
	cairo_paint (cr);

	cairo_destroy (cr);

	return FALSE;
}

gboolean
bitmap_button_button_press ( GtkWidget *widget, GdkEventButton *event )
{
	if (event->type == GDK_BUTTON_PRESS && event->button == 1)
	{
		bitmap_button *self = g_object_get_data (G_OBJECT (widget), bitmap_button_key); g_assert (self);
		g_signal_emit_by_name(self->adjustment, "start_atomic_value_change");
		gdouble value = gtk_adjustment_get_value (self->adjustment);
		gdouble lower = gtk_adjustment_get_lower (self->adjustment);
		gdouble upper = gtk_adjustment_get_upper (self->adjustment);
		gdouble middl = (upper - lower) / 2.0;
		gtk_adjustment_set_value (self->adjustment, (value < middl) ? 1.0 : 0.0);
		return TRUE;
	}
	return FALSE;
}

static void
bitmap_button_update (GtkWidget *widget)
{
	bitmap_button *self = g_object_get_data (G_OBJECT (widget), bitmap_button_key); g_assert (self);
	
	gdouble value = gtk_adjustment_get_value (self->adjustment);
	gdouble lower = gtk_adjustment_get_lower (self->adjustment);
	gdouble upper = gtk_adjustment_get_upper (self->adjustment);
	guint	frame = (guint) (self->frame_count * ((value - lower) / (upper - lower)));

	self->current_frame = MIN (frame, (self->frame_count - 1));
	
	gtk_widget_queue_draw (widget);
}

static void
bitmap_button_adjustment_changed			( GtkAdjustment *adjustment, gpointer data )
{
	bitmap_button_update (data);
}

static void
bitmap_button_adjustment_value_changed	( GtkAdjustment *adjustment, gpointer data )
{
	bitmap_button_update (data);
}

static void
bitmap_button_set_adjustment( GtkWidget *widget, GtkAdjustment *adjustment )
{
	bitmap_button *self = g_object_get_data (G_OBJECT (widget), bitmap_button_key); g_assert (self);

	if (self->adjustment)
	{
		g_signal_handlers_disconnect_by_data (GTK_OBJECT (self->adjustment), (gpointer) self);
		g_object_unref (GTK_OBJECT (self->adjustment) );
	}
	
	self->adjustment = GTK_ADJUSTMENT (g_object_ref (GTK_OBJECT (adjustment)));

	g_signal_connect (GTK_OBJECT (adjustment), "changed",
		(GCallback) bitmap_button_adjustment_changed,
		(gpointer) widget );
		
	g_signal_connect (GTK_OBJECT (adjustment), "value_changed",
		(GCallback) bitmap_button_adjustment_value_changed,
		(gpointer) widget );
	
	bitmap_button_adjustment_changed (adjustment, widget);
}

////////////////////////////////////////////////////////////////////////////////
