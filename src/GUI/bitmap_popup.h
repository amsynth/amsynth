
#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

GtkWidget *bitmap_popup_new (
	GtkAdjustment *,
	GdkPixbuf *,
	guint frame_width,
	guint frame_height,
	guint frame_count
);

void bitmap_popup_set_bg (
	GtkWidget *widget,
	GdkPixbuf *background
);

void bitmap_popup_set_strings (
	GtkWidget	*widget,
	const gchar	**strings
);

G_END_DECLS

