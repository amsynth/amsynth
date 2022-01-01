/*
 *  amsynth_lv2_ui_gtk.c
 *
 *  Copyright (c) 2001-2021 Nick Dowell
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

#include "amsynth_lv2.h"

#include "controls.h"
#include "GUI/editor_pane.h"
#include "Preset.h"
#include "Synthesizer.h"

// works around an issue in qtractor version <= 0.5.6
// http://sourceforge.net/p/qtractor/tickets/19/
#define CALL_LV2UI_WRITE_FUNCTION_ON_IDLE 1

////////////////////////////////////////////////////////////////////////////////

typedef struct {
	GtkWidget *_widget;
	GtkAdjustment *_adjustments[kAmsynthParameterCount];
#if CALL_LV2UI_WRITE_FUNCTION_ON_IDLE
	gboolean _adjustment_changed[kAmsynthParameterCount];
#endif
	gboolean _dont_send_control_changes;
	LV2_Atom_Forge forge;
	LV2_URID_Map *map;
	LV2UI_Write_Function _write_function;
	LV2UI_Controller _controller;
	guint _timeout_id;

	struct {
		LV2_URID atom_Float;
		LV2_URID atom_Path;
		LV2_URID atom_Resource;
		LV2_URID atom_Sequence;
		LV2_URID atom_URID;
		LV2_URID atom_eventTransfer;
		LV2_URID amsynth_kbm_file;
		LV2_URID amsynth_scl_file;
		LV2_URID midi_Event;
		LV2_URID patch_Get;
		LV2_URID patch_Set;
		LV2_URID patch_property;
		LV2_URID patch_value;
	} uris;
} lv2_ui;

static void on_adjustment_value_changed(GtkAdjustment *adjustment, gpointer user_data);

#if CALL_LV2UI_WRITE_FUNCTION_ON_IDLE
static gboolean lv2_ui_on_idle(gpointer data)
{
	lv2_ui *ui = (lv2_ui *) data;
	if (!ui->_write_function)
		return TRUE;

	size_t i; for (i = 0; i<kAmsynthParameterCount; i++) {
		if (ui->_adjustment_changed[i] && ui->_adjustments[i]) {
			float value = gtk_adjustment_get_value(ui->_adjustments[i]);
			ui->_write_function(ui->_controller,
				PORT_FIRST_PARAMETER + i,
				sizeof(float), 0, &value);
		}
	}

	return TRUE;
}
#endif

////////////////////////////////////////////////////////////////////////////////

struct SynthesizerStub : ISynthesizer
{
	SynthesizerStub(lv2_ui *ui_): ui(ui_) {}

	int loadTuningKeymap(const char *filename) override
	{
		send(ui->uris.amsynth_kbm_file, filename ?: "");
		return 0;
	}

	int loadTuningScale(const char *filename) override
	{
		send(ui->uris.amsynth_scl_file, filename ?: "");
		return 0;
	}

	void send(LV2_URID key, const char *value)
	{
		uint8_t buffer[1024];

		LV2_Atom_Forge_Frame frame;
		LV2_Atom_Forge *forge = &ui->forge;
		lv2_atom_forge_set_buffer(forge, buffer, sizeof(buffer));
		LV2_Atom *msg = (LV2_Atom *) lv2_atom_forge_object(forge, &frame, 0, ui->uris.patch_Set);
		lv2_atom_forge_key(forge, ui->uris.patch_property);
		lv2_atom_forge_urid(forge, key);
		lv2_atom_forge_key(forge, ui->uris.patch_value);
		lv2_atom_forge_path(forge, value, (uint32_t) strlen(value));
		lv2_atom_forge_pop(forge, &frame);

		ui->_write_function(
				ui->_controller,
				PORT_CONTROL,
				lv2_atom_total_size(msg),
				ui->uris.atom_eventTransfer,
				msg);
	}

	lv2_ui *ui;
};

////////////////////////////////////////////////////////////////////////////////

static LV2UI_Handle
lv2_ui_instantiate(const LV2UI_Descriptor* descriptor,
				   const char*                     plugin_uri,
				   const char*                     bundle_path,
				   LV2UI_Write_Function            write_function,
				   LV2UI_Controller                controller,
				   LV2UI_Widget*                   widget,
				   const LV2_Feature* const*       features)
{
	lv2_ui *ui = (lv2_ui *) g_malloc0 (sizeof(lv2_ui));

	const char *missing = lv2_features_query(
			features,
			LV2_URID__map, &ui->map, true,
			NULL);
	if (missing) {
		free(ui);
		return nullptr;
	}

	ui->uris.atom_Float         = ui->map->map(ui->map->handle, LV2_ATOM__Float);
	ui->uris.atom_Path          = ui->map->map(ui->map->handle, LV2_ATOM__Path);
	ui->uris.atom_Resource      = ui->map->map(ui->map->handle, LV2_ATOM__Resource);
	ui->uris.atom_Sequence      = ui->map->map(ui->map->handle, LV2_ATOM__Sequence);
	ui->uris.atom_URID          = ui->map->map(ui->map->handle, LV2_ATOM__URID);
	ui->uris.atom_eventTransfer = ui->map->map(ui->map->handle, LV2_ATOM__eventTransfer);
	ui->uris.amsynth_kbm_file   = ui->map->map(ui->map->handle, AMSYNTH__tuning_kbm_file);
	ui->uris.amsynth_scl_file   = ui->map->map(ui->map->handle, AMSYNTH__tuning_scl_file);
	ui->uris.midi_Event         = ui->map->map(ui->map->handle, LV2_MIDI__MidiEvent);
	ui->uris.patch_Get          = ui->map->map(ui->map->handle, LV2_PATCH__Get);
	ui->uris.patch_Set          = ui->map->map(ui->map->handle, LV2_PATCH__Set);
	ui->uris.patch_property     = ui->map->map(ui->map->handle, LV2_PATCH__property);
	ui->uris.patch_value        = ui->map->map(ui->map->handle, LV2_PATCH__value);

	ui->_write_function = write_function;
	ui->_controller = controller;

	lv2_atom_forge_init(&ui->forge, ui->map);

	size_t i; for (i=0; i<kAmsynthParameterCount; i++) {
		gdouble value = 0, lower = 0, upper = 0, step_increment = 0;
		get_parameter_properties(i, &lower, &upper, &value, &step_increment);
		ui->_adjustments[i] = (GtkAdjustment *)gtk_adjustment_new(value, lower, upper, step_increment, 0, 0);
		g_object_ref_sink(ui->_adjustments[i]); // assumes ownership of the floating reference
		g_signal_connect(ui->_adjustments[i], "value-changed", (GCallback)&on_adjustment_value_changed, ui);
	}

	ui->_widget = editor_pane_new(new SynthesizerStub(ui), ui->_adjustments, TRUE, 0);

	*widget = ui->_widget;

#if CALL_LV2UI_WRITE_FUNCTION_ON_IDLE
    ui->_timeout_id = g_timeout_add_full(G_PRIORITY_LOW, 1000/60, (GSourceFunc)&lv2_ui_on_idle, ui, nullptr);
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
	lv2_ui *ui = (lv2_ui *) user_data;
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
					PORT_FIRST_PARAMETER + i,
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
	int parameter_index = port_index - PORT_FIRST_PARAMETER;
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
	nullptr
};

LV2_SYMBOL_EXPORT
const LV2UI_Descriptor *
lv2ui_descriptor(uint32_t index)
{
	if (index == 0) {
		return &descriptor;
	}
	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////
