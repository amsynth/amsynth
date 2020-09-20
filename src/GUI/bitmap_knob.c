/*
 *  bitmap_knob.c
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

#include "bitmap_knob.h"

#include "../controls.h"

#include <math.h>
#include <stdio.h>

////////////////////////////////////////////////////////////////////////////////

#define SENSITIVITY_STEP     40 // Pixels required to step up to next value
#define SENSITIVITY_NORMAL  300 // Pixels required to travel full range
#define SENSITIVITY_SCROLL   10 // Number of scroll wheel clicks to travel full range

////////////////////////////////////////////////////////////////////////////////

typedef struct {

	GtkWidget *drawing_area;
	GtkWidget *tooltip_window;
	GtkWidget *tooltip_label;

	GtkAdjustment *adjustment;
	unsigned long parameter_index;
	
	GdkPixbuf *pixbuf;
	GdkPixbuf *background;
	gint current_frame;
	gint frame_width;
	gint frame_height;
	gint frame_count;
	gint scaling_factor;
	
	gdouble origin_y;
	gdouble origin_val;
	
} bitmap_knob;

static const gchar *bitmap_knob_key = "bitmap_knob";

////////////////////////////////////////////////////////////////////////////////

static gboolean bitmap_knob_expose			( GtkWidget *wigdet, GdkEventExpose *event );
static gboolean bitmap_knob_button_press	( GtkWidget *wigdet, GdkEventButton *event );
static gboolean bitmap_knob_button_release	( GtkWidget *wigdet, GdkEventButton *event );
static gboolean bitmap_knob_motion_notify	( GtkWidget *wigdet, GdkEventMotion *event );
static gboolean bitmap_knob_scroll		( GtkWidget *wigdet, GdkEventScroll *event );

static void		bitmap_knob_set_adjustment				( GtkWidget *widget, GtkAdjustment *adjustment );
static void		bitmap_knob_adjustment_changed			( GtkAdjustment *adjustment, gpointer data );
static void		bitmap_knob_adjustment_value_changed	( GtkAdjustment *adjustment, gpointer data );

////////////////////////////////////////////////////////////////////////////////

GtkWidget *
bitmap_knob_new( GtkAdjustment *adjustment,
                 GdkPixbuf *pixbuf,
                 gint frame_width,
                 gint frame_height,
                 gint frame_count,
				 gint scaling_factor)
{
	bitmap_knob *self = g_malloc0 (sizeof(bitmap_knob));

	self->drawing_area = gtk_drawing_area_new ();
	self->pixbuf		= g_object_ref (pixbuf);
	self->frame_width	= frame_width;
	self->frame_height	= frame_height;
	self->frame_count	= frame_count;
	self->scaling_factor = scaling_factor;

	g_object_set_data_full (G_OBJECT (self->drawing_area), bitmap_knob_key, self, (GDestroyNotify) g_free);
	g_assert (g_object_get_data (G_OBJECT (self->drawing_area), bitmap_knob_key));
	
	g_signal_connect (G_OBJECT (self->drawing_area), "expose-event", G_CALLBACK (bitmap_knob_expose), NULL);
	g_signal_connect (G_OBJECT (self->drawing_area), "button-press-event", G_CALLBACK (bitmap_knob_button_press), NULL);
	g_signal_connect (G_OBJECT (self->drawing_area), "button-release-event", G_CALLBACK (bitmap_knob_button_release), NULL);
	g_signal_connect (G_OBJECT (self->drawing_area), "motion-notify-event", G_CALLBACK (bitmap_knob_motion_notify), NULL);
	g_signal_connect (G_OBJECT (self->drawing_area), "scroll-event", G_CALLBACK (bitmap_knob_scroll), NULL);

	gtk_widget_set_size_request (self->drawing_area, frame_width * scaling_factor, frame_height * scaling_factor);
	
	// set up event mask
	gint event_mask = gtk_widget_get_events (self->drawing_area);
	event_mask |= GDK_BUTTON_PRESS_MASK;
	event_mask |= GDK_BUTTON_RELEASE_MASK;
	event_mask |= GDK_BUTTON1_MOTION_MASK;
	event_mask |= GDK_SCROLL_MASK;
	gtk_widget_set_events (self->drawing_area, event_mask);
	
	bitmap_knob_set_adjustment (self->drawing_area, adjustment);

	/* tooltip */

	self->tooltip_window = gtk_window_new (GTK_WINDOW_POPUP);
	gtk_window_set_type_hint (GTK_WINDOW (self->tooltip_window), GDK_WINDOW_TYPE_HINT_TOOLTIP);
	g_object_set_data_full (G_OBJECT(self->drawing_area), "bitmap_knob_tooltip_window",
		self->tooltip_window, (GDestroyNotify) gtk_widget_destroy);

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

	self->background = pixbuf ? GDK_PIXBUF (g_object_ref (G_OBJECT (pixbuf))) : NULL;

	gtk_widget_queue_draw (widget);
}

void bitmap_knob_set_parameter_index (GtkWidget *widget, unsigned long parameter_index)
{
	bitmap_knob *self = g_object_get_data (G_OBJECT (widget), bitmap_knob_key);
	self->parameter_index = parameter_index;
}

////////////////////////////////////////////////////////////////////////////////

static int tooltip_update (bitmap_knob *self)
{
	float value = (float) gtk_adjustment_get_value (self->adjustment);
	char display[32] = "";
	int result = parameter_get_display ((int) self->parameter_index, value, display, sizeof(display));
	gtk_label_set_text (GTK_LABEL (self->tooltip_label), display);
	return result;
}

static void tooltip_show (bitmap_knob *self)
{
	gtk_widget_show (self->tooltip_window);

	gint widget_x = 0, widget_y = 0;
	gdk_window_get_origin (gtk_widget_get_window (self->drawing_area), &widget_x, &widget_y);

	gint tooltip_height = 0;
	gdk_window_get_geometry (gtk_widget_get_window (self->tooltip_window), NULL, NULL, NULL, &tooltip_height, NULL);

	gint width = 0;
	gint height = 0;
	gtk_widget_get_size_request (self->drawing_area, &width, &height);

	gint tooltip_x = widget_x + width + 4;
	gint tooltop_y = widget_y + (height - tooltip_height) / 2;

	gtk_window_move (GTK_WINDOW (self->tooltip_window), tooltip_x, tooltop_y);

	return;
}

////////////////////////////////////////////////////////////////////////////////

static gboolean
bitmap_knob_expose( GtkWidget *widget, GdkEventExpose *event )
{
	bitmap_knob *self = g_object_get_data (G_OBJECT (widget), bitmap_knob_key);
	
	cairo_t *cr = gdk_cairo_create (event->window);

	cairo_scale (cr, self->scaling_factor, self->scaling_factor);

	if (self->background) {
		gdk_cairo_set_source_pixbuf (cr, self->background, 0, 0);
		// CAIRO_EXTEND_NONE results in a ugly border when upscaling
		cairo_pattern_t *pattern = cairo_get_source (cr);
		cairo_pattern_set_extend (pattern, CAIRO_EXTEND_PAD);
		cairo_paint (cr);
	}
	
	gint src_x = 0, src_y = 0;
	
	if (gdk_pixbuf_get_height (self->pixbuf) == self->frame_height)
		src_x = self->current_frame * self->frame_width;
	else
		src_y = self->current_frame * self->frame_height;

	gdk_cairo_set_source_pixbuf (cr, self->pixbuf, -src_x, -src_y);
	cairo_paint (cr);

	cairo_destroy (cr);

	return FALSE;
}

gboolean
bitmap_knob_button_press ( GtkWidget *widget, GdkEventButton *event )
{
	bitmap_knob *self = g_object_get_data (G_OBJECT (widget), bitmap_knob_key);

	if (event->type == GDK_2BUTTON_PRESS) {
		GValue *value = g_object_get_data (G_OBJECT(self->adjustment), "default-value");
		float defaultValue = g_value_get_float(value);
		gtk_adjustment_set_value (self->adjustment, defaultValue);
		return TRUE;
	}

	if (event->type == GDK_BUTTON_PRESS && event->button == 1)
	{
		gtk_widget_grab_focus(widget);
		gtk_grab_add(widget);
		g_signal_emit_by_name(self->adjustment, "start_atomic_value_change");
		self->origin_val = gtk_adjustment_get_value (self->adjustment);
		self->origin_y = event->y;
		if (tooltip_update (self))
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
		if (gtk_widget_has_grab(widget))
			gtk_grab_remove(widget);
		return TRUE;
	}
	return FALSE;
}

gboolean
bitmap_knob_motion_notify ( GtkWidget *widget, GdkEventMotion *event )
{
	if (gtk_widget_has_grab(widget)) {
		bitmap_knob *self = g_object_get_data (G_OBJECT (widget), bitmap_knob_key);
		gdouble lower = gtk_adjustment_get_lower (self->adjustment);
		gdouble upper = gtk_adjustment_get_upper (self->adjustment);
		gdouble step  = gtk_adjustment_get_step_increment (self->adjustment);
		gdouble range = upper - lower;
		guint sensitivity = SENSITIVITY_NORMAL;
		if (step == 0.0) {
			if (event->state & GDK_SHIFT_MASK) {
				sensitivity *= 4;
			}
			if (event->state & GDK_CONTROL_MASK) {
				sensitivity *= 4;
			}
		} else {
			sensitivity = SENSITIVITY_STEP * (guint)((range) / step);
			sensitivity = MIN(sensitivity, 480);
		}
		gdouble offset = (self->origin_y - event->y) * range / sensitivity;
		gdouble newval = self->origin_val + ((step == 0.0) ? offset : step * floor ((offset / step) + 0.5));
		if (newval != self->origin_val) {
			gtk_adjustment_set_value (self->adjustment, CLAMP (newval, lower, upper));
			self->origin_val = gtk_adjustment_get_value (self->adjustment);
			self->origin_y = event->y;
			tooltip_update (self);
		}
		return TRUE;
	}
	return FALSE;
}

gboolean
bitmap_knob_scroll ( GtkWidget *widget, GdkEventScroll *event )
{
	bitmap_knob *self = g_object_get_data (G_OBJECT (widget), bitmap_knob_key);

	int increment = 0;	// assume neither UP nor DOWN
	if (event->direction == GDK_SCROLL_UP) {
		increment = 1;
	}
	if (event->direction == GDK_SCROLL_DOWN) {
		increment = -1;
	}

	if (increment) {
		gdouble lower = gtk_adjustment_get_lower (self->adjustment);
		gdouble upper = gtk_adjustment_get_upper (self->adjustment);
		gdouble step  = gtk_adjustment_get_step_increment (self->adjustment);
		gdouble value = gtk_adjustment_get_value (self->adjustment);
		gdouble range = upper - lower;
		gdouble newval = value + range * increment / (double)SENSITIVITY_SCROLL;
		gtk_adjustment_set_value (self->adjustment, CLAMP (newval, lower, upper));
		tooltip_update (self);
		return TRUE;
	}

	return FALSE;
}

static void
bitmap_knob_update ( GtkWidget *widget )
{
	bitmap_knob *self = g_object_get_data (G_OBJECT (widget), bitmap_knob_key);
	
	gdouble value = gtk_adjustment_get_value (self->adjustment);
	gdouble lower = gtk_adjustment_get_lower (self->adjustment);
	gdouble upper = gtk_adjustment_get_upper (self->adjustment);
	gint	frame = (gint) ((self->frame_count - 1) * ((value - lower) / (upper - lower)));

	frame = MIN (frame, (self->frame_count - 1));

	if (self->current_frame != frame) {
		self->current_frame = frame;
		gtk_widget_queue_draw (widget);
	}
}

static void
bitmap_knob_adjustment_changed			( GtkAdjustment *adjustment, gpointer data )
{
	bitmap_knob_update (data);
}

static void
bitmap_knob_adjustment_value_changed	( GtkAdjustment *adjustment, gpointer data )
{
	bitmap_knob_update (data);
}

static void
bitmap_knob_set_adjustment( GtkWidget *widget, GtkAdjustment *adjustment )
{
	bitmap_knob *self = g_object_get_data (G_OBJECT (widget), bitmap_knob_key);

	if (self->adjustment)
	{
		g_signal_handlers_disconnect_by_data (GTK_OBJECT (self->adjustment), (gpointer) self);
		g_object_unref (GTK_OBJECT (self->adjustment) );
	}
	
	self->adjustment = GTK_ADJUSTMENT (g_object_ref (GTK_OBJECT (adjustment)));
	
	g_signal_connect (GTK_OBJECT (adjustment), "changed",
		(GCallback) bitmap_knob_adjustment_changed,
		(gpointer) widget );
		
	g_signal_connect (GTK_OBJECT (adjustment), "value_changed",
		(GCallback) bitmap_knob_adjustment_value_changed,
		(gpointer) widget );
	
	bitmap_knob_adjustment_changed (adjustment, widget);
}

////////////////////////////////////////////////////////////////////////////////

