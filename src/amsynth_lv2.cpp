/*
 *  amsynth_lv2.cpp
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

#include "amsynth_lv2.h"

#include "Preset.h"
#include "Synthesizer.h"

#include <cstdio>
#include <map>
#include <string>

#ifdef DEBUG
#define LOG_FUNCTION_CALL()		fprintf(stderr, AMSYNTH_LV2_URI " %s\n", __FUNCTION__)
#else
#define LOG_FUNCTION_CALL()
#endif

struct amsynth_wrapper {
	amsynth_wrapper() : schedule(nullptr), control_port(nullptr), out_l(nullptr), out_r(nullptr) {}

	Synthesizer synth;

	struct {
		LV2_URID midiEvent;
		LV2_URID patch_Set;
		LV2_URID patch_property;
		LV2_URID patch_value;
		LV2_URID atom_String;
		LV2_URID amsynth_kbm_file;
		LV2_URID amsynth_scl_file;
	} uris;

	LV2_Atom_Forge forge;
	LV2_Worker_Schedule *schedule;

	const LV2_Atom_Sequence *control_port;
	float *out_l;
	float *out_r;
	float *param_ports[kAmsynthParameterCount];

	std::map<LV2_URID, std::string> patch_values;

	void patchSet(LV2_URID urid, const char *value)
	{
		patch_values[urid] = (std::string) value;

		if (urid == uris.amsynth_kbm_file)
			synth.loadTuningKeymap(value);

		if (urid == uris.amsynth_scl_file)
			synth.loadTuningScale(value);
	}
};

static LV2_Handle
lv2_instantiate(const LV2_Descriptor *descriptor, double sample_rate, const char *bundle_path, const LV2_Feature *const *features)
{
	LOG_FUNCTION_CALL();

	amsynth_wrapper *a = new amsynth_wrapper;

	LV2_URID_Map *urid_map = nullptr;
	const char *missing = lv2_features_query(
			features,
			LV2_URID__map,        &urid_map,      true,
			LV2_WORKER__schedule, &a->schedule,   true,
			NULL);
	if (missing) {
		free(a);
		return nullptr;
	}

	a->synth.setSampleRate((int)sample_rate);

	a->uris.midiEvent          = urid_map->map(urid_map->handle, LV2_MIDI__MidiEvent);
	a->uris.patch_Set          = urid_map->map(urid_map->handle, LV2_PATCH__Set);
	a->uris.patch_property     = urid_map->map(urid_map->handle, LV2_PATCH__property);
	a->uris.patch_value        = urid_map->map(urid_map->handle, LV2_PATCH__value);
	a->uris.atom_String        = urid_map->map(urid_map->handle, LV2_ATOM__String);
	a->uris.amsynth_kbm_file   = urid_map->map(urid_map->handle, AMSYNTH__tuning_kbm_file);
	a->uris.amsynth_scl_file   = urid_map->map(urid_map->handle, AMSYNTH__tuning_scl_file);

	lv2_atom_forge_init(&a->forge, urid_map);

	return (LV2_Handle) a;
}

static void
lv2_cleanup(LV2_Handle instance)
{
	LOG_FUNCTION_CALL();

	delete (amsynth_wrapper *) instance;
}

static void
lv2_connect_port(LV2_Handle instance, uint32_t port, void *data_location)
{
	amsynth_wrapper * a = (amsynth_wrapper *) instance;
	switch (port) {
		case PORT_CONTROL:
			a->control_port = (LV2_Atom_Sequence *) data_location;
			break;
		case PORT_NOTIFY:
			break;
		case PORT_AUDIO_L:
			a->out_l = (float *) data_location;
			break;
		case PORT_AUDIO_R:
			a->out_r = (float *) data_location;
			break;
		default:
			if (PORT_FIRST_PARAMETER <= port && (port - PORT_FIRST_PARAMETER) < kAmsynthParameterCount) {
				a->param_ports[port - PORT_FIRST_PARAMETER] = (float *) data_location;
			}
			break;
	}
}

static void
lv2_activate(LV2_Handle instance)
{
	LOG_FUNCTION_CALL();
}

static void
lv2_deactivate(LV2_Handle instance)
{
	LOG_FUNCTION_CALL();
}

static void
lv2_run(LV2_Handle instance, uint32_t sample_count)
{
	amsynth_wrapper * a = (amsynth_wrapper *) instance;

	std::vector<amsynth_midi_event_t> midi_events;
	LV2_ATOM_SEQUENCE_FOREACH(a->control_port, ev) {
		if (ev->body.type == a->uris.midiEvent) {
			amsynth_midi_event_t midi_event = {0};
			midi_event.offset_frames = ev->time.frames;
			midi_event.buffer = (uint8_t *)(ev + 1);
			midi_event.length = ev->body.size;
			midi_events.push_back(midi_event);
		}
		if (lv2_atom_forge_is_object_type(&a->forge, ev->body.type)) {
			const LV2_Atom_Object *obj = (const LV2_Atom_Object *) &ev->body;
			if (obj->body.otype == a->uris.patch_Set) {
				a->schedule->schedule_work(a->schedule->handle, lv2_atom_total_size(&ev->body), &ev->body);
			}
		}
	}

	for (unsigned i=0; i<kAmsynthParameterCount; i++) {
		const float *host_value = a->param_ports[i];
		if (host_value != nullptr) {
			if (a->synth.getParameterValue((Param)i) != *host_value) {
				a->synth.setParameterValue((Param)i, *host_value);
			}
		}
	}

	std::vector<amsynth_midi_cc_t> midi_out;
	a->synth.process(sample_count, midi_events, midi_out, a->out_l, a->out_r);
}

static LV2_State_Status
save(LV2_Handle                instance,
     LV2_State_Store_Function  store,
     LV2_State_Handle          handle,
     uint32_t                  flags,
     const LV2_Feature* const* features)
{
	LOG_FUNCTION_CALL();

	amsynth_wrapper * a = (amsynth_wrapper *) instance;

	// host takes care of saving port values

	std::map<LV2_URID, std::string>::const_iterator it;
	for (it = a->patch_values.begin(); it != a->patch_values.end(); ++it) {
		if (!it->second.length())
			continue;
		store(handle, it->first, it->second.c_str(), it->second.length() + 1,
			  a->uris.atom_String, LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);
	}

	return LV2_STATE_SUCCESS;
}

static LV2_State_Status
restore(LV2_Handle                  instance,
        LV2_State_Retrieve_Function retrieve,
        LV2_State_Handle            handle,
        uint32_t                    flags,
        const LV2_Feature* const*   features)
{
	LOG_FUNCTION_CALL();

	amsynth_wrapper * a = (amsynth_wrapper *) instance;

	// host takes care of restoring port values

	LV2_URID urids[] = { a->uris.amsynth_kbm_file, a->uris.amsynth_scl_file };
	for (unsigned i = 0; i < sizeof(urids) / sizeof(urids[0]); i ++) {
		size_t size = 0; uint32_t type = 0, vflags = 0;
		const void *value = retrieve(handle, urids[i], &size, &type, &vflags);
		if (value && type == a->uris.atom_String) {
			a->patchSet(urids[i], (const char *) value);
		}
	}

	return LV2_STATE_SUCCESS;
}

static LV2_Worker_Status
work(LV2_Handle                  instance,
	 LV2_Worker_Respond_Function respond,
	 LV2_Worker_Respond_Handle   handle,
	 uint32_t                    size,
	 const void*                 data)
{
	LOG_FUNCTION_CALL();

	amsynth_wrapper * a = (amsynth_wrapper *) instance;

	const LV2_Atom_Object *obj = (const LV2_Atom_Object *) data;
	if (obj->body.otype == a->uris.patch_Set) {
		const LV2_Atom *property = nullptr;
		const LV2_Atom *value = nullptr;
		lv2_atom_object_get(obj,
							a->uris.patch_property, &property,
							a->uris.patch_value, &value,
							0);
		a->patchSet(((LV2_Atom_URID *) (void *) property)->body, (const char *) LV2_ATOM_BODY_CONST(value));
	}

	return LV2_WORKER_SUCCESS;
}

static LV2_Worker_Status
work_response(LV2_Handle  instance,
			  uint32_t    size,
			  const void* data)
{
	LOG_FUNCTION_CALL();

	return LV2_WORKER_SUCCESS;
}

static const void *
lv2_extension_data(const char *uri)
{
	LOG_FUNCTION_CALL();

	if (strcmp(uri, LV2_STATE__interface) == 0) {
		static const LV2_State_Interface state = { save, restore };
		return &state;
	}

	if (strcmp(uri, LV2_WORKER__interface) == 0) {
		static const LV2_Worker_Interface worker = { work, work_response, nullptr };
		return &worker;
	}

	return nullptr;
}

static const LV2_Descriptor amsynth1_descriptor = {
	AMSYNTH_LV2_URI,
	&lv2_instantiate,
	&lv2_connect_port,
	&lv2_activate,
	&lv2_run,
	&lv2_deactivate,
	&lv2_cleanup,
	&lv2_extension_data
};

LV2_SYMBOL_EXPORT
const LV2_Descriptor *
lv2_descriptor(uint32_t index)
{
	LOG_FUNCTION_CALL();

	switch (index) {
	case 0:
		return &amsynth1_descriptor;
	default:
		return nullptr;
	}
}
