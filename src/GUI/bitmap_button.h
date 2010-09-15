
#pragma once

#include <gtk/gtk.h>

G_BEGIN_DECLS

GtkWidget *bitmap_button_new (
	GtkAdjustment *,
	GdkPixbuf *,
	guint frame_width,
	guint frame_height,
	guint frame_count
);

void bitmap_button_set_bg (
	GtkWidget *widget,
	GdkPixbuf *background
);

G_END_DECLS
