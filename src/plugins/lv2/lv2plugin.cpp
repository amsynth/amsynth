/*
 *  lv2plugin.cpp
 *
 *  Copyright (c) 2012 Nick Dowell
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

#include "lv2plugin.h"

#include "core/synth/Preset.h"
#include "core/synth/Synthesizer.h"

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
		LV2_URID patch_Get;
		LV2_URID patch_Set;
		LV2_URID patch_property;
		LV2_URID patch_value;
		LV2_URID atom_String;
#define DECLARE_LV2_URID(name) LV2_URID amsynth_##name;
		FOR_EACH_PROPERTY(DECLARE_LV2_URID)
	} uris;

	LV2_Atom_Forge forge;
	LV2_Worker_Schedule *schedule;

	const LV2_Atom_Sequence *control_port;
    LV2_Atom_Sequence *notify_port {nullptr};
	float *out_l;
	float *out_r;
	float *param_ports[kAmsynthParameterCount];

	std::map<LV2_URID, std::string> patch_values;

	void patchSet(LV2_URID urid, const char *value)
	{
		patch_values[urid] = (std::string) value;
#define PATCH_SET_PROP(Name) if (urid == uris.amsynth_##Name) synth.setProperty(#Name, value);
		FOR_EACH_PROPERTY(PATCH_SET_PROP)
	}
};

static LV2_Handle
lv2_instantiate(const LV2_Descriptor *, double sample_rate, const char */*bundle_path*/, const LV2_Feature *const *features)
{
	LOG_FUNCTION_CALL();

	amsynth_wrapper *a = new amsynth_wrapper;

	LV2_URID_Map *urid_map = nullptr;
	for (auto f = features; *f; f++) {
		if (!strcmp((*f)->URI, LV2_URID__map))
			urid_map = reinterpret_cast<LV2_URID_Map *>((*f)->data);
		if (!strcmp((*f)->URI, LV2_WORKER__schedule))
			a->schedule = reinterpret_cast<LV2_Worker_Schedule *>((*f)->data);
	}
	if (!urid_map) {
		delete a;
		return nullptr;
	}

	a->synth.setSampleRate((int)sample_rate);

	a->uris.midiEvent          = urid_map->map(urid_map->handle, LV2_MIDI__MidiEvent);
	a->uris.patch_Get          = urid_map->map(urid_map->handle, LV2_PATCH__Get);
	a->uris.patch_Set          = urid_map->map(urid_map->handle, LV2_PATCH__Set);
	a->uris.patch_property     = urid_map->map(urid_map->handle, LV2_PATCH__property);
	a->uris.patch_value        = urid_map->map(urid_map->handle, LV2_PATCH__value);
	a->uris.atom_String        = urid_map->map(urid_map->handle, LV2_ATOM__String);
#define MAP_URID(Name) a->uris.amsynth_##Name = urid_map->map(urid_map->handle, AMSYNTH_LV2_URI "#" #Name);
	FOR_EACH_PROPERTY(MAP_URID)

	auto properties = a->synth.getProperties();
#define GET_PATCH_VALUE(Name) a->patch_values[a->uris.amsynth_##Name] = properties[#Name];
	FOR_EACH_PROPERTY(GET_PATCH_VALUE)

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
            a->notify_port = (LV2_Atom_Sequence *)data_location;
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
lv2_activate(LV2_Handle)
{
	LOG_FUNCTION_CALL();
}

static void
lv2_deactivate(LV2_Handle)
{
	LOG_FUNCTION_CALL();
}

static void
lv2_run(LV2_Handle instance, uint32_t sample_count)
{
	amsynth_wrapper * a = (amsynth_wrapper *) instance;
	LV2_Atom_Forge *forge = &a->forge;

    // Set up forge to write directly to notify output port.
    const uint32_t notify_capacity = a->notify_port->atom.size;
    lv2_atom_forge_set_buffer(forge, (uint8_t *)a->notify_port, notify_capacity);

    // Start a sequence in the notify output port.
    LV2_Atom_Forge_Frame notify_frame;
    lv2_atom_forge_sequence_head(forge, &notify_frame, 0);

    std::vector<amsynth_midi_event_t> midi_events;
	LV2_ATOM_SEQUENCE_FOREACH(a->control_port, ev) {
		if (ev->body.type == a->uris.midiEvent) {
			amsynth_midi_event_t midi_event {};
			midi_event.offset_frames = static_cast<unsigned>(ev->time.frames);
			midi_event.buffer = (uint8_t *)(ev + 1);
			midi_event.length = ev->body.size;
			midi_events.push_back(midi_event);
		}
		if (lv2_atom_forge_is_object_type(forge, ev->body.type)) {
			const auto *obj = (const LV2_Atom_Object *)&ev->body;
			if (obj->body.otype == a->uris.patch_Get) {
                // UI has requested a property value
				const LV2_Atom_URID *property = nullptr;
				lv2_atom_object_get(obj, a->uris.patch_property, &property, 0);
				if (!property)
					continue;
				const LV2_URID key = property->body;
				auto it = a->patch_values.find(key);
				if (it == a->patch_values.end())
					continue;
				lv2_atom_forge_frame_time(forge, ev->time.frames);
				LV2_Atom_Forge_Frame frame;
				lv2_atom_forge_object(forge, &frame, 0, a->uris.patch_Set);
				lv2_atom_forge_key(forge, a->uris.patch_property);
				lv2_atom_forge_urid(forge, key);
				lv2_atom_forge_key(forge, a->uris.patch_value);
				lv2_atom_forge_string(forge, it->second.c_str(), static_cast<uint32_t>(it->second.size()));
				lv2_atom_forge_pop(forge, &frame);
			}
			if (obj->body.otype == a->uris.patch_Set) {
                // Note: some property types could be handled here without using worker
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
     uint32_t                  /*flags*/,
     const LV2_Feature* const* /*features*/)
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
        uint32_t                    /*flags*/,
        const LV2_Feature* const*   /*features*/)
{
	LOG_FUNCTION_CALL();

	amsynth_wrapper * a = (amsynth_wrapper *) instance;

	// host takes care of restoring port values

	auto restoreProp = [=] (LV2_URID urid) {
		size_t size = 0; uint32_t type = 0, vflags = 0;
		const void *value = retrieve(handle, urid, &size, &type, &vflags);
		if (value && type == a->uris.atom_String) {
			a->patchSet(urid, (const char *)value);
		}
	};

#define RESTORE_PROP(name) restoreProp(a->uris.amsynth_##name);
	FOR_EACH_PROPERTY(RESTORE_PROP)

	return LV2_STATE_SUCCESS;
}

static LV2_Worker_Status
work(LV2_Handle                  instance,
	 LV2_Worker_Respond_Function /*respond*/,
	 LV2_Worker_Respond_Handle   /*handle*/,
	 uint32_t                    /*size*/,
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
work_response(LV2_Handle  /*instance*/,
			  uint32_t    /*size*/,
			  const void* /*data*/)
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
