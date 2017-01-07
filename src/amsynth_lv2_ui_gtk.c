/*
 *  amsynth_lv2_ui_gtk.c
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

////////////////////////////////////////////////////////////////////////////////

#include "controls.h"
#include "Preset.h"
#include "GUI/editor_pane.h"

#include "lv2/lv2plug.in/ns/lv2core/lv2.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"

#include <stdio.h>

// works around an issue in qtractor version <= 0.5.6
// http://sourceforge.net/p/qtractor/tickets/19/
#define CALL_LV2UI_WRITE_FUNCTION_ON_IDLE 1

////////////////////////////////////////////////////////////////////////////////

enum {
	kAmsynthPortIndexForFirstParameter = 3,
};

typedef struct {
	GtkWidget *_widget;
	GtkAdjustment *_adjustments[kAmsynthParameterCount];
#if CALL_LV2UI_WRITE_FUNCTION_ON_IDLE
	gboolean _adjustment_changed[kAmsynthParameterCount];
#endif
	gboolean _dont_send_control_changes;
	LV2UI_Write_Function _write_function;
	LV2UI_Controller _controller;
	guint _timeout_id;
} lv2_ui;

static void on_adjustment_value_changed(GtkAdjustment *adjustment, gpointer user_data);

#if CALL_LV2UI_WRITE_FUNCTION_ON_IDLE
static gboolean lv2_ui_on_idle(gpointer data)
{
	lv2_ui *ui = data;
	if (!ui->_write_function)
		return TRUE;

	size_t i; for (i = 0; i<kAmsynthParameterCount; i++) {
		if (ui->_adjustment_changed[i] && ui->_adjustments[i]) {
			float value = gtk_adjustment_get_value(ui->_adjustments[i]);
			ui->_write_function(ui->_controller,
				kAmsynthPortIndexForFirstParameter + i,
				sizeof(float), 0, &value);
		}
	}

	return TRUE;
}
#endif

////////////////////////////////////////////////////////////////////////////////

static LV2UI_Handle
lv2_ui_instantiate(const struct _LV2UI_Descriptor* descriptor,
				   const char*                     plugin_uri,
				   const char*                     bundle_path,
				   LV2UI_Write_Function            write_function,
				   LV2UI_Controller                controller,
				   LV2UI_Widget*                   widget,
				   const LV2_Feature* const*       features)
{
	lv2_ui *ui = g_malloc0 (sizeof(lv2_ui));
	ui->_write_function = write_function;
	ui->_controller = controller;

	size_t i; for (i=0; i<kAmsynthParameterCount; i++) {
		gdouble value = 0, lower = 0, upper = 0, step_increment = 0;
		get_parameter_properties(i, &lower, &upper, &value, &step_increment);
		ui->_adjustments[i] = (GtkAdjustment *)gtk_adjustment_new(value, lower, upper, step_increment, 0, 0);
		g_object_ref_sink(ui->_adjustments[i]); // assumes ownership of the floating reference
		g_signal_connect(ui->_adjustments[i], "value-changed", (GCallback)&on_adjustment_value_changed, ui);
	}

	ui->_widget = editor_pane_new(ui->_adjustments, TRUE);

	*widget = ui->_widget;

#if CALL_LV2UI_WRITE_FUNCTION_ON_IDLE
    ui->_timeout_id = g_timeout_add_full(G_PRIORITY_LOW, 1000/60, (GSourceFunc)&lv2_ui_on_idle, ui, NULL);
#endif

	return ui;
}

static void
lv2_ui_cleanup(LV2UI_Handle ui)
{
#if CALL_LV2UI_WRITE_FUNCTION_ON_IDLE
	g_source_remove(((lv2_ui *)ui)->_timeout_id);
#endif
	size_t i; for (i=0; i<kAmsynthParameterCount; i++) {
		g_object_unref (((lv2_ui *)ui)->_adjustments[i]);
	}
	g_free (ui);
}

static void
on_adjustment_value_changed(GtkAdjustment *adjustment, gpointer user_data)
{
	lv2_ui *ui = user_data;
	if (ui->_dont_send_control_changes)
		return;

	size_t i; for (i = 0; i<kAmsynthParameterCount; i++) {
		if (adjustment == ui->_adjustments[i]) {
#if CALL_LV2UI_WRITE_FUNCTION_ON_IDLE
			ui->_adjustment_changed[i] = TRUE;
#else
			float value = gtk_adjustment_get_value(adjustment);
			if (ui->_write_function != 0) {
				ui->_write_function(ui->_controller,
					kAmsynthPortIndexForFirstParameter + i,
					sizeof(float), 0, &value);
			}
#endif
			break;
		}
	}
}

static void
lv2_ui_port_event(LV2UI_Handle ui,
				  uint32_t     port_index,
				  uint32_t     buffer_size,
				  uint32_t     format,
				  const void*  buffer)
{
	int parameter_index = port_index - kAmsynthPortIndexForFirstParameter;
	if (parameter_index < 0 || parameter_index >= kAmsynthParameterCount)
		return;
	float value = *(float *)buffer;
	GtkAdjustment *adjustment = ((lv2_ui *)ui)->_adjustments[parameter_index];
	((lv2_ui *)ui)->_dont_send_control_changes = TRUE;
	gtk_adjustment_set_value(adjustment, value);
#if CALL_LV2UI_WRITE_FUNCTION_ON_IDLE
	((lv2_ui *)ui)->_adjustment_changed[parameter_index] = FALSE;
#endif
	((lv2_ui *)ui)->_dont_send_control_changes = FALSE;
}

////////////////////////////////////////////////////////////////////////////////

void modal_midi_learn(Param param_index)
{
}

////////////////////////////////////////////////////////////////////////////////

static const
LV2UI_Descriptor descriptor = {
	"http://code.google.com/p/amsynth/amsynth/ui/gtk",
	&lv2_ui_instantiate,
	&lv2_ui_cleanup,
	&lv2_ui_port_event,
	0
};

LV2_SYMBOL_EXPORT
const LV2UI_Descriptor *
lv2ui_descriptor(uint32_t index)
{
	if (index == 0) {
		return &descriptor;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
