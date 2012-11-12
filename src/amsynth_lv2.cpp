
#include "lv2/lv2plug.in/ns/ext/atom/util.h"
#include "lv2/lv2plug.in/ns/ext/state/state.h"
#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/ext/worker/worker.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/lv2core/lv2.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "controls.h"
#include "MidiController.h"
#include "PresetController.h"
#include "VoiceAllocationUnit.h"

#define AMSYNTH_LV2_URI			"http://code.google.com/p/amsynth/amsynth"

#define LOG_ERROR(msg)			fprintf(stderr, AMSYNTH_LV2_URI " error: " msg "\n")
#ifdef DEBUG
#define LOG_FUNCTION_CALL()		fprintf(stderr, AMSYNTH_LV2_URI " %s\n", __FUNCTION__)
#else
#define LOG_FUNCTION_CALL()
#endif

struct amsynth_wrapper {
	const char *bundle_path;
	VoiceAllocationUnit *vau;
	PresetController *bank;
	MidiController *mc;
	float * out_l;
	float * out_r;
	const LV2_Atom_Sequence *midi_in_port;
	float ** params;
	struct {
		LV2_URID midiEvent;
	} uris;
};

static LV2_Handle
lv2_instantiate(const struct _LV2_Descriptor *descriptor, double sample_rate, const char *bundle_path, const LV2_Feature *const *features)
{
	LOG_FUNCTION_CALL();

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
	a->uris.midiEvent       = urid_map->map(urid_map->handle, LV2_MIDI__MidiEvent);

	return (LV2_Handle) a;
}

static void
lv2_cleanup(LV2_Handle instance)
{
	LOG_FUNCTION_CALL();

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
	case 2: a->midi_in_port = (LV2_Atom_Sequence *)data_location; break;
	default:
		if ((port - 3) < kAmsynthParameterCount) { a->params[port-3] = (float *)data_location; }
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

	LV2_ATOM_SEQUENCE_FOREACH(a->midi_in_port, ev) {
		if (ev->body.type == a->uris.midiEvent) {
			uint32_t size = ev->body.size;
			uint8_t *data = (uint8_t *)(ev + 1);
			a->mc->HandleMidiData(data, size);
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

static LV2_State_Status
save(LV2_Handle                instance,
     LV2_State_Store_Function  store,
     LV2_State_Handle          handle,
     uint32_t                  flags,
     const LV2_Feature* const* features)
{
	LOG_FUNCTION_CALL();

	// host takes care of saving port values
	// we have no additional state to save

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
	// we have no additional state to restore
	
	return LV2_STATE_SUCCESS;
}

static const void *
lv2_extension_data(const char *uri)
{
	LOG_FUNCTION_CALL();

	if (!strcmp(uri, LV2_STATE__interface)) {
		static const LV2_State_Interface state = { save, restore };
		return &state;
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
