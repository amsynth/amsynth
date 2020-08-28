/*
 *  bitmap_popup.c
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

#include "bitmap_popup.h"

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
	gint scaling_factor;
	
	GtkWidget *menu;

} bitmap_popup;

static const gchar *bitmap_popup_key = "bitmap_popup";

////////////////////////////////////////////////////////////////////////////////

static gboolean bitmap_popup_expose			( GtkWidget *wigdet, GdkEventExpose *event );
static gboolean bitmap_popup_button_release ( GtkWidget *wigdet, GdkEventButton *event );

static void		bitmap_popup_set_adjustment				( GtkWidget *widget, GtkAdjustment *adjustment );
static void		bitmap_popup_adjustment_changed			( GtkAdjustment *adjustment, gpointer data );
static void		bitmap_popup_adjustment_value_changed	( GtkAdjustment *adjustment, gpointer data );
static void		bitmap_popup_menuitem_activated			( GtkWidget *menu_item, gpointer data );

////////////////////////////////////////////////////////////////////////////////

GtkWidget *
bitmap_popup_new( GtkAdjustment *adjustment,
				 GdkPixbuf *pixbuf,
				 gint frame_width,
				 gint frame_height,
				 gint frame_count,
				 gint scaling_factor)
{
	bitmap_popup *self = g_malloc0 (sizeof(bitmap_popup));

	self->drawing_area = gtk_drawing_area_new ();
	self->pixbuf		= g_object_ref (pixbuf);
	self->frame_width	= frame_width;
	self->frame_height	= frame_height;
	self->frame_count	= frame_count;
	self->scaling_factor = scaling_factor;

	g_object_set_data_full (G_OBJECT (self->drawing_area), bitmap_popup_key, self, (GDestroyNotify) g_free);
	g_assert (g_object_get_data (G_OBJECT (self->drawing_area), bitmap_popup_key));
	
	g_signal_connect (G_OBJECT (self->drawing_area), "expose-event", G_CALLBACK (bitmap_popup_expose), NULL);

	g_signal_connect (G_OBJECT (self->drawing_area), "button-release-event", G_CALLBACK (bitmap_popup_button_release), NULL);

	gtk_widget_set_size_request (self->drawing_area, frame_width * scaling_factor, frame_height * scaling_factor);
	
	// set up event mask
	gint event_mask = gtk_widget_get_events (self->drawing_area);
	event_mask |= GDK_BUTTON_PRESS_MASK;
	event_mask |= GDK_BUTTON_RELEASE_MASK;
	gtk_widget_set_events (self->drawing_area, event_mask);
	
	bitmap_popup_set_adjustment (self->drawing_area, adjustment);
	
	return self->drawing_area;
}

void bitmap_popup_set_bg (GtkWidget *widget, GdkPixbuf *pixbuf)
{
	bitmap_popup *self = g_object_get_data (G_OBJECT (widget), bitmap_popup_key);

	if (self->background) {
		g_object_unref (G_OBJECT (self->background));
	}

	self->background = pixbuf ? GDK_PIXBUF (g_object_ref (G_OBJECT (pixbuf))) : NULL;

	gtk_widget_queue_draw (widget);
}

void bitmap_popup_set_strings (GtkWidget *widget, const gchar **strings)
{
	bitmap_popup *self = g_object_get_data (G_OBJECT (widget), bitmap_popup_key);

	g_assert (!self->menu);

	self->menu = gtk_menu_new ();
	
	gtk_menu_attach_to_widget (GTK_MENU (self->menu), widget, NULL);
	
	gint i;
	GSList *group = NULL;
	const gint min = (gint)gtk_adjustment_get_lower (self->adjustment);
	const gint max = (gint)gtk_adjustment_get_upper (self->adjustment);
	for (i = min; i <= max; i++)
	{
		gchar *label = g_strstrip (g_strdup(strings[i - min]));
		GtkWidget *menu_item = gtk_radio_menu_item_new_with_label (group, label);
		group = gtk_radio_menu_item_get_group (GTK_RADIO_MENU_ITEM (menu_item));
		g_signal_connect (GTK_OBJECT (menu_item), "activate",
			(GCallback) bitmap_popup_menuitem_activated,
			(gpointer) self );
		gtk_menu_shell_append (GTK_MENU_SHELL(self->menu), menu_item);
		g_object_unref (G_OBJECT (menu_item));
		g_free (label);
	}
	
	gtk_widget_show_all (self->menu);
}

////////////////////////////////////////////////////////////////////////////////

static gboolean
bitmap_popup_expose( GtkWidget *widget, GdkEventExpose *event )
{
	bitmap_popup *self = g_object_get_data (G_OBJECT (widget), bitmap_popup_key);

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
bitmap_popup_button_release ( GtkWidget *widget, GdkEventButton *event )
{
	bitmap_popup *self = g_object_get_data (G_OBJECT (widget), bitmap_popup_key);
	gint min = (gint)gtk_adjustment_get_lower (self->adjustment);
	gint max = (gint)gtk_adjustment_get_upper (self->adjustment);
	gint val = (gint)gtk_adjustment_get_value (self->adjustment);
	gint i; for (i = min; i <= max; i++) {
		if (i == val) {
			GList *items = gtk_container_get_children (GTK_CONTAINER (self->menu));
			GtkWidget *menu_item = g_list_nth_data (items, (guint) (i - min));
			gtk_check_menu_item_set_active (GTK_CHECK_MENU_ITEM (menu_item), TRUE);
			break;
		}
	}
	g_signal_emit_by_name(self->adjustment, "start_atomic_value_change");
	gtk_menu_popup (GTK_MENU (self->menu), NULL, NULL, NULL, NULL, event->button, event->time);
	return TRUE;
}

void
bitmap_popup_menuitem_activated (GtkWidget *menu_item, gpointer data)
{
	bitmap_popup *self = data;
	
	GList *list = gtk_container_get_children (GTK_CONTAINER (self->menu));
	int i = g_list_index (list, menu_item);
	g_list_free (list);
	
	gdouble lower = gtk_adjustment_get_lower (self->adjustment);
	gtk_adjustment_set_value (self->adjustment, lower + i);
}

static void
bitmap_popup_update (GtkWidget *widget)
{
	bitmap_popup *self = g_object_get_data (G_OBJECT (widget), bitmap_popup_key);
	
	gdouble value = gtk_adjustment_get_value (self->adjustment);
	gdouble lower = gtk_adjustment_get_lower (self->adjustment);
	gdouble upper = gtk_adjustment_get_upper (self->adjustment);
	gint	frame = (gint) ((self->frame_count - 1) * ((value - lower) / (upper - lower)));

	self->current_frame = MIN (frame, (self->frame_count - 1));
	
	gtk_widget_queue_draw (widget);
}

static void
bitmap_popup_adjustment_changed			( GtkAdjustment *adjustment, gpointer data )
{
	bitmap_popup_update (data);
}

static void
bitmap_popup_adjustment_value_changed	( GtkAdjustment *adjustment, gpointer data )
{
	bitmap_popup_update (data);
}

static void
bitmap_popup_set_adjustment( GtkWidget *widget, GtkAdjustment *adjustment )
{
	bitmap_popup *self = g_object_get_data (G_OBJECT (widget), bitmap_popup_key);

	if (self->adjustment)
	{
		g_signal_handlers_disconnect_by_data (GTK_OBJECT (self->adjustment), (gpointer) self);
		g_object_unref (GTK_OBJECT (self->adjustment) );
	}
	
	self->adjustment = GTK_ADJUSTMENT (g_object_ref (GTK_OBJECT (adjustment)));

	g_signal_connect (GTK_OBJECT (adjustment), "changed",
		(GCallback) bitmap_popup_adjustment_changed,
		(gpointer) widget );
		
	g_signal_connect (GTK_OBJECT (adjustment), "value_changed",
		(GCallback) bitmap_popup_adjustment_value_changed,
		(gpointer) widget );
	
	bitmap_popup_adjustment_changed (adjustment, widget);
}

////////////////////////////////////////////////////////////////////////////////

