/*
 *  editor_pane.h
 *
 *  Copyright (c) 2001-2012 Nick Dowell
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

#ifndef _editor_pane_h
#define _editor_pane_h

#include "../controls.h"

#include <gtk/gtk.h>

G_BEGIN_DECLS

GtkWidget *
editor_pane_new(void *synthesizer, GtkAdjustment **adjustments, gboolean is_plugin, int scaling_factor);

void modal_midi_learn(Param param_index);

G_END_DECLS

#endif
