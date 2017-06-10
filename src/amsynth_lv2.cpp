/*
 *  amsynth_lv2.cpp
 *
 *  Copyright (c) 2001-2017 Nick Dowell
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

#include "lv2/lv2plug.in/ns/ext/atom/forge.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/patch/patch.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"
#include "lv2/lv2plug.in/ns/ext/worker/worker.h"
#include "lv2/lv2plug.in/ns/lv2core/lv2_util.h"

#ifdef DEBUG
#define LOG_FUNCTION_CALL()		fprintf(stderr, AMSYNTH_LV2_URI " %s\n", __FUNCTION__)
#else
#define LOG_FUNCTION_CALL()
#endif

struct amsynth_wrapper {
	const char *bundle_path;
	Synthesizer *synth;
	float * out_l;
	float * out_r;

	const LV2_Atom_Sequence *midi_in_port;
	const LV2_Atom_Sequence *control_port;
	LV2_Atom_Forge forge;
	LV2_Worker_Schedule *schedule;

	float ** params;
	struct {
		LV2_URID midiEvent;
		LV2_URID patch_Set;
		LV2_URID patch_property;
		LV2_URID patch_value;
		LV2_URID amsynth_kbm_file;
		LV2_URID amsynth_scl_file;
	} uris;
};

static LV2_Handle
lv2_instantiate(const struct _LV2_Descriptor *descriptor, double sample_rate, const char *bundle_path, const LV2_Feature *const *features)
{
	LOG_FUNCTION_CALL();

	amsynth_wrapper *a = (amsynth_wrapper *)calloc(1, sizeof(amsynth_wrapper));

	LV2_URID_Map *urid_map = NULL;
	const char *missing = lv2_features_query(
			features,
			LV2_URID__map,        &urid_map,      true,
			LV2_WORKER__schedule, &a->schedule,   true,
			NULL);
	if (missing) {
		free(a);
		return NULL;
	}

	a->bundle_path = strdup(bundle_path);
	a->synth = new Synthesizer;
	a->synth->setSampleRate((int)sample_rate);
	a->params = (float **) calloc (kAmsynthParameterCount, sizeof (float *));

	a->uris.midiEvent          = urid_map->map(urid_map->handle, LV2_MIDI__MidiEvent);
	a->uris.patch_Set          = urid_map->map(urid_map->handle, LV2_PATCH__Set);
	a->uris.patch_property     = urid_map->map(urid_map->handle, LV2_PATCH__property);
	a->uris.patch_value        = urid_map->map(urid_map->handle, LV2_PATCH__value);
	a->uris.amsynth_kbm_file   = urid_map->map(urid_map->handle, AMSYNTH__tuning_kbm_file);
	a->uris.amsynth_scl_file   = urid_map->map(urid_map->handle, AMSYNTH__tuning_scl_file);

	lv2_atom_forge_init(&a->forge, urid_map);

	return (LV2_Handle) a;
}

static void
lv2_cleanup(LV2_Handle instance)
{
	LOG_FUNCTION_CALL();

	amsynth_wrapper * a = (amsynth_wrapper *) instance;
	free ((void *)a->bundle_path);
	delete a->synth;
	free (a->params);
	free ((void *)a);
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
		case PORT_MIDI_IN:
			a->midi_in_port = (LV2_Atom_Sequence *) data_location;
			break;
		default:
			if (PORT_FIRST_PARAMETER >= port && (port - PORT_FIRST_PARAMETER) < kAmsynthParameterCount) {
				a->params[port - PORT_FIRST_PARAMETER] = (float *) data_location;
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
	LV2_ATOM_SEQUENCE_FOREACH(a->midi_in_port, ev) {
		if (ev->body.type == a->uris.midiEvent) {
			amsynth_midi_event_t midi_event = {0};
			midi_event.offset_frames = ev->time.frames;
			midi_event.buffer = (uint8_t *)(ev + 1);
			midi_event.length = ev->body.size;
			midi_events.push_back(midi_event);
		}
	}

	LV2_ATOM_SEQUENCE_FOREACH(a->control_port, ev) {
		if (lv2_atom_forge_is_object_type(&a->forge, ev->body.type)) {
			const LV2_Atom_Object *obj = (const LV2_Atom_Object *) &ev->body;
			if (obj->body.otype == a->uris.patch_Set) {
				a->schedule->schedule_work(a->schedule->handle, lv2_atom_total_size(&ev->body), &ev->body);
			}
		}
	}

	for (unsigned i=0; i<kAmsynthParameterCount; i++) {
		const float *host_value = a->params[i];
		if (host_value != NULL) {
			if (a->synth->getParameterValue((Param)i) != *host_value) {
				a->synth->setParameterValue((Param)i, *host_value);
			}
		}
	}

	std::vector<amsynth_midi_cc_t> midi_out;
	a->synth->process(sample_count, midi_events, midi_out, a->out_l, a->out_r);
}

static LV2_State_Status
save(LV2_Handle                instance,
     LV2_State_Store_Function  store,
     LV2_State_Handle          handle,
     uint32_t                  flags,
     const LV2_Feature* const* features)
{
	LOG_FUNCTION_CALL();

	// host takes care of saving port values

	// TODO: store current tuning setup

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

	// host takes care of restoring port values

	// TODO: restore current tuning setup

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
		const LV2_Atom *property = NULL;
		const LV2_Atom *value = NULL;
		lv2_atom_object_get(obj,
							a->uris.patch_property, &property,
							a->uris.patch_value, &value,
							0);

		LV2_URID urid = ((LV2_Atom_URID *) (void *) property)->body;
		const char *filename = (const char *) LV2_ATOM_BODY_CONST(value);

		fprintf(stderr, "amsynth lv2 worker: urid=%d filename=%s\n", urid, filename);

		if (urid == a->uris.amsynth_kbm_file) {
			if (strlen(filename)) {
				a->synth->loadTuningKeymap(filename);
			} else {
				a->synth->defaultTuning();
			}
		}

		if (urid == a->uris.amsynth_scl_file) {
			if (strlen(filename)) {
				a->synth->loadTuningScale(filename);
			} else {
				a->synth->defaultTuning();
			}
		}
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
		static const LV2_Worker_Interface worker = { work, work_response, NULL };
		return &worker;
	}

	return NULL;
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
		return NULL;
	}
}
