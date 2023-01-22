/*
 *  lv2ui.c
 *
 *  Copyright (c) 2001-2023 Nick Dowell
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

#include "lv2plugin.h"

#include "core/controls.h"
#include "core/gui/ControlPanel.h"
#include "core/gui/juce_x11.h"
#include "core/synth/Preset.h"
#include "core/synth/Synthesizer.h"

#include <cstring>
#include <memory>
#include <utility>

////////////////////////////////////////////////////////////////////////////////

struct ParameterListener final : public UpdateListener {
	ParameterListener(PresetController *presetController, std::function<void(int, float)> writeFunc)
	: presetController(presetController), writeFunc(std::move(writeFunc)) {
		presetController->getCurrentPreset().AddListenerToAll(this);
		active = true;
	}

	~ParameterListener() final {
		for (int i = 0; i < kAmsynthParameterCount; i++) {
			presetController->getCurrentPreset().getParameter(i).removeUpdateListener(this);
		}
	}

	void UpdateParameter(Param param, float controlValue) override {
		if (!active) return;
		writeFunc(param, presetController->getCurrentPreset().getParameter(param).getValue());
	}

	PresetController *presetController;
	std::function<void(int, float)> writeFunc;
	bool active{false};
};

////////////////////////////////////////////////////////////////////////////////

typedef struct {
	PresetController presetController;
	std::unique_ptr<ControlPanel> controlPanel;
	std::unique_ptr<ParameterListener> parameterListener;
	LV2UI_Widget parent {nullptr};

	LV2_Atom_Forge forge;
	LV2_URID_Map *map;
	LV2UI_Write_Function _write_function;
	LV2UI_Controller _controller;

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
};

////////////////////////////////////////////////////////////////////////////////

struct lv2helper
{
	lv2helper(lv2_ui *ui_): ui(ui_) {}

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
	lv2_ui *ui = new lv2_ui();

	LV2UI_Resize *resize {nullptr};

	for (auto f = features; *f; f++) {
		if (!strcmp((*f)->URI, LV2_UI__parent))
			ui->parent = reinterpret_cast<LV2UI_Widget>((*f)->data);
		if (!strcmp((*f)->URI, LV2_URID__map))
			ui->map = reinterpret_cast<LV2_URID_Map *>((*f)->data);
		if (!strcmp((*f)->URI, LV2_UI__resize))
			resize = reinterpret_cast<LV2UI_Resize *>((*f)->data);
	}

	if (!ui->map) {
		delete ui;
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

	ui->parameterListener = std::make_unique<ParameterListener>(&ui->presetController, [=] (int idx, float value) {
		write_function(controller, PORT_FIRST_PARAMETER + idx, sizeof(float), 0, &value);
	});

	juceInit();

	ui->controlPanel = std::make_unique<ControlPanel>(&ui->presetController, true);
	ui->controlPanel->loadTuningKbm = [ui] (auto f) { lv2helper(ui)->loadTuningKeymap(f); };
	ui->controlPanel->loadTuningScl = [ui] (auto f) { lv2helper(ui)->loadTuningScale(f); };
	ui->controlPanel->addToDesktop(juce::ComponentPeer::windowIgnoresKeyPresses, ui->parent);
	ui->controlPanel->setVisible(true);
	if (resize) {
		auto bounds = ui->controlPanel->getScreenBounds();
		auto scaleFactor = (int)juce::Desktop::getInstance().getGlobalScaleFactor();
		resize->ui_resize(resize->handle, bounds.getWidth() * (int)scaleFactor, bounds.getHeight() * (int)scaleFactor);
	}
	*widget = ui->controlPanel->getWindowHandle();

	return ui;
}

static void
lv2_ui_cleanup(LV2UI_Handle ui)
{
	((lv2_ui *)ui)->controlPanel->removeFromDesktop();
	delete ((lv2_ui *)ui);
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
	((lv2_ui *)ui)->presetController.getCurrentPreset().getParameter(paramId).setValue(value);
}

////////////////////////////////////////////////////////////////////////////////

static int
lv2_ui_idle(LV2UI_Handle ui)
{
	juceIdle();
	return 0;
}

static const void *
lv2_ui_extension_data(const char *uri)
{
	if (!strcmp(uri, LV2_UI__idleInterface)) {
		static LV2UI_Idle_Interface idleInterface {
			.idle = &lv2_ui_idle
		};
		return &idleInterface;
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////

void modal_midi_learn(Param param_index)
{
}

////////////////////////////////////////////////////////////////////////////////

static const
LV2UI_Descriptor descriptor = {
	"http://code.google.com/p/amsynth/amsynth/x11ui",
	&lv2_ui_instantiate,
	&lv2_ui_cleanup,
	&lv2_ui_port_event,
	&lv2_ui_extension_data,
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
