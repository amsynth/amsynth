#ifndef _editor_pane_h
#define _editor_pane_h

#include <gtk/gtk.h>

G_BEGIN_DECLS

GtkWidget * editor_pane_new (GtkAdjustment **adjustments);

void modal_midi_learn(int param_index);

G_END_DECLS

#endif

