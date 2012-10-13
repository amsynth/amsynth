
#include <lv2/lv2plug.in/ns/lv2core/lv2.h>
#include <lv2/lv2plug.in/ns/ext/atom/atom.h>
#include <lv2/lv2plug.in/ns/ext/event/event-helpers.h>
#include <lv2/lv2plug.in/ns/ext/event/event.h>
#include <lv2/lv2plug.in/ns/ext/midi/midi.h>
#include <lv2/lv2plug.in/ns/ext/state/state.h>
#include <lv2/lv2plug.in/ns/ext/urid/urid.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "controls.h"
#include "MidiController.h"
#include "PresetController.h"
#include "VoiceAllocationUnit.h"

#define AMSYNTH_LV2_URI			"http://code.google.com/p/amsynth/amsynth"
#define AMSYNTH_LV2__Preset		AMSYNTH_LV2_URI "#preset"

#define LOG_ERROR(msg) fprintf(stderr, AMSYNTH_LV2_URI " error: " msg "\n")

struct amsynth_wrapper {
	const char *bundle_path;
	VoiceAllocationUnit *vau;
	PresetController *bank;
	MidiController *mc;
	float * out_l;
	float * out_r;
	LV2_Event_Buffer *midi_in_port;
	float ** params;
	struct {
		LV2_URID atom;
		LV2_URID midiEvent;
	} uris;
};

static LV2_Handle
lv2_instantiate(const struct _LV2_Descriptor *descriptor, double sample_rate, const char *bundle_path, const LV2_Feature *const *features)
{
	LV2_URID_Map *urid_map = NULL;
	for (int i = 0; features[i]; ++i) {
		if (!strcmp(features[i]->URI, LV2_URID__map)) {
			urid_map = (LV2_URID_Map *)features[i]->data;
		}
	}
	if (urid_map == NULL) {
		LOG_ERROR("host does not support " LV2_URID__map);
		return NULL;
	}
	if (!urid_map->map(urid_map->handle, LV2_ATOM__Atom)) {
		LOG_ERROR("host does not support " LV2_ATOM__Atom);
		return NULL;
	}
	if (!urid_map->map(urid_map->handle, LV2_MIDI__MidiEvent)) {
		LOG_ERROR("host does not support " LV2_MIDI__MidiEvent);
		return NULL;
	}

	static Config config;
	config.Defaults();
	Preset amsynth_preset;

	amsynth_wrapper *a = (amsynth_wrapper *)calloc(1, sizeof(amsynth_wrapper));
	a->bundle_path = strdup(bundle_path);
	a->vau = new VoiceAllocationUnit;
	a->vau->SetSampleRate (sample_rate);
	a->bank = new PresetController;
	a->bank->loadPresets(config.current_bank_file.c_str());
	a->bank->selectPreset(0);
	a->bank->getCurrentPreset().AddListenerToAll (a->vau);
	a->mc = new MidiController(config);
	a->mc->SetMidiEventHandler(a->vau);
	a->mc->setPresetController(*a->bank);
	a->mc->set_midi_channel(0);
	a->params = (float **) calloc (kAmsynthParameterCount, sizeof (float *));
	a->uris.atom        = urid_map->map(urid_map->handle, LV2_ATOM__Atom);
	a->uris.midiEvent   = urid_map->map(urid_map->handle, LV2_MIDI__MidiEvent);

	return (LV2_Handle) a;
}

static void
lv2_cleanup(LV2_Handle instance)
{
	amsynth_wrapper * a = (amsynth_wrapper *) instance;
	free ((void *)a->bundle_path);
	delete a->vau;
	delete a->bank;
	free (a->params);
	free ((void *)a);
}

static void
lv2_connect_port(LV2_Handle instance, uint32_t port, void *data_location)
{
	amsynth_wrapper * a = (amsynth_wrapper *) instance;
	switch (port) {
	case 0: a->out_l = (float *)data_location; break;
	case 1: a->out_r = (float *)data_location; break;
	case 2: a->midi_in_port = (LV2_Event_Buffer *)data_location; break;
	default:
		if ((port - 3) < kAmsynthParameterCount) { a->params[port-3] = (float *)data_location; }
		break;
	}
}

static void
lv2_activate(LV2_Handle instance)
{
}

static void
lv2_deactivate(LV2_Handle instance)
{
}

static void
lv2_run(LV2_Handle instance, uint32_t sample_count)
{
	amsynth_wrapper * a = (amsynth_wrapper *) instance;

	if (a->midi_in_port != NULL) {
		LV2_Event_Iterator ev_it;
		lv2_event_begin(&ev_it, a->midi_in_port);
		const uint32_t event_count = ev_it.buf->event_count;
		for (uint32_t i=0; i < event_count; ++i) {
			uint8_t *data = NULL;
			LV2_Event *ev = lv2_event_get(&ev_it, &data);
			if (ev->type == a->uris.midiEvent) {
				a->mc->HandleMidiData(data, ev->size);
			}
			lv2_event_increment(&ev_it);
		}
	}

	for (unsigned i=0; i<kAmsynthParameterCount; i++) {
		const float *host_value = a->params[i];
		if (host_value != NULL) {
			if (a->bank->getCurrentPreset().getParameter(i).getValue() != *host_value) {
				a->bank->getCurrentPreset().getParameter(i).setValue(*host_value);
			}
		}
	}

	a->vau->Process (a->out_l, a->out_r, sample_count);
}

static const void *
lv2_extension_data(const char *uri)
{
	return NULL;
}

static const LV2_Descriptor amsynth1_descriptor = {
	"http://code.google.com/p/amsynth/amsynth",
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
	switch (index) {
	case 0:
		return &amsynth1_descriptor;
	default:
		return NULL;
	}
}
