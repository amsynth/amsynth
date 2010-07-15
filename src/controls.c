
#include "controls.h"

#define SIZEOF_ARRAY( a ) ( sizeof((a)) / sizeof((a)[0]) )

static const char *param_names[] = {
	"amp_attack",
	"amp_decay",
	"amp_sustain",
	"amp_release",
	"osc1_waveform",
	"filter_attack",
	"filter_decay",
	"filter_sustain",
	"filter_release",
	"filter_resonance",
	"filter_env_amount",
	"filter_cutoff",
	"osc2_detune",
	"osc2_waveform",
	"master_vol",
	"lfo_freq",
	"lfo_waveform",
	"osc2_range",
	"osc_mix",
	"freq_mod_amount",
	"filter_mod_amount",
	"amp_mod_amount",
	"osc_mix_mode",
	"osc1_pulsewidth",
	"osc2_pulsewidth",
	"reverb_roomsize",
	"reverb_damp",
	"reverb_wet",
	"reverb_width",
	"distortion_crunch",
	"osc2_sync",
};

const char *parameter_name_from_index (int param_index)
{
	return param_names[param_index];
}

int parameter_index_from_name (const char *param_name)
{
	int i;
	for (i=0; i<SIZEOF_ARRAY(param_names); i++) {
		if (strcmp (param_name, param_names[i]) == 0) {
			return i;
		}
	}
	return -1;
}


