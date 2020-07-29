/*
 *  amsynth_dssi.cpp
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

#include "amsynth_dssi.h"

#include "midi.h"
#include "Preset.h"
#include "PresetController.h"
#include "Synthesizer.h"

#include <assert.h>
#include <climits>
#include <dssi.h>
#include <ladspa.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#ifdef DEBUG
#define TRACE( msg ) fprintf (stderr, "[amsynth-dssi] %s(): " msg "\n", __func__)
#define TRACE_ARGS( fmt, ... ) fprintf (stderr, "[amsynth-dssi] %s(): " fmt "\n", __func__, __VA_ARGS__)
#else
#define TRACE( msg ) (void)0
#define TRACE_ARGS( ... ) (void)0
#endif

static LADSPA_Descriptor *	s_ladspaDescriptor = nullptr;
static DSSI_Descriptor *	s_dssiDescriptor   = nullptr;
static PresetController *	s_presetController = nullptr;
static unsigned long 		s_lastBankGet = ULONG_MAX;

#define MIDI_BUFFER_SIZE 4096

typedef struct _amsynth_wrapper {
	Synthesizer *synth;
	unsigned char *midi_buffer;
	LADSPA_Data *out_l;
	LADSPA_Data *out_r;
	LADSPA_Data **params;
} amsynth_wrapper;




/*	DLL Entry point
 *
 *- Each shared object file containing DSSI plugins must include a
 *   function dssi_descriptor(), with the following function prototype
 *   and C-style linkage.  Hosts may enumerate the plugin types
 *   available in the shared object file by repeatedly calling
 *   this function with successive Index values (beginning from 0),
 *   until a return value of NULL indicates no more plugin types are
 *   available.  Each non-NULL return is the DSSI_Descriptor
 *   of a distinct plugin type.
 */
__attribute__((visibility("default")))
const DSSI_Descriptor *dssi_descriptor (unsigned long index)
{
    switch (index)
    {
    case 0: return s_dssiDescriptor;
    default: return nullptr;
    }
}




//////////////////// Constructor & Destructor (equivalents) ////////////////////

static LADSPA_Handle instantiate (const LADSPA_Descriptor * descriptor, unsigned long s_rate)
{
	TRACE();
    amsynth_wrapper * a = new amsynth_wrapper;
    a->synth = new Synthesizer;
    a->synth->setSampleRate(s_rate);
    a->midi_buffer = (unsigned char *)calloc(MIDI_BUFFER_SIZE, 1);
    a->params = (LADSPA_Data **) calloc (kAmsynthParameterCount, sizeof (LADSPA_Data *));
    return (LADSPA_Handle) a;
}

static void cleanup (LADSPA_Handle instance)
{
	TRACE();
    amsynth_wrapper * a = (amsynth_wrapper *) instance;
    delete a->synth;
    free (a->midi_buffer);
    free (a->params);
    delete a;
}

static char * configure(LADSPA_Handle Instance, const char *Key, const char *Value)
{
	Synthesizer *synthesizer = ((amsynth_wrapper *) Instance)->synth;

	if (strcmp(Key, PROP_KBM_FILE) == 0) {
		synthesizer->loadTuningKeymap(Value);
		return nullptr;
	}

	if (strcmp(Key, PROP_SCL_FILE) == 0) {
		synthesizer->loadTuningScale(Value);
		return nullptr;
	}

	return nullptr;
}

//////////////////// Program handling //////////////////////////////////////////

static const DSSI_Program_Descriptor *get_program(LADSPA_Handle Instance, unsigned long Index)
{
	static DSSI_Program_Descriptor descriptor;
	memset(&descriptor, 0, sizeof(descriptor));

	descriptor.Program = Index % PresetController::kNumPresets;
	descriptor.Bank = Index / PresetController::kNumPresets;

	const std::vector<BankInfo> &banks = PresetController::getPresetBanks();
	if (descriptor.Bank < banks.size())
	{
		if (descriptor.Bank != s_lastBankGet) {
			s_presetController->loadPresets(banks[descriptor.Bank].file_path.c_str());
			s_lastBankGet = descriptor.Bank;
		}
		descriptor.Name = s_presetController->getPreset(descriptor.Program).getName().c_str();
		TRACE_ARGS("%d %d %s", descriptor.Bank, descriptor.Program, descriptor.Name);
		return &descriptor;
	}

	return nullptr;
}

static void select_program(LADSPA_Handle Instance, unsigned long Bank, unsigned long Index)
{
	amsynth_wrapper * a = (amsynth_wrapper *) Instance;

	TRACE_ARGS("Bank = %d Index = %d", Bank, Index);

	const std::vector<BankInfo> &banks = PresetController::getPresetBanks();

	if (Bank < banks.size() && Index < PresetController::kNumPresets) {
		s_presetController->loadPresets(banks[Bank].file_path.c_str());
		a->synth->setPresetNumber(Index);
		// now update DSSI host's view of the parameters
		for (unsigned i = 0; i < kAmsynthParameterCount; i++) {
			float value = a->synth->getParameterValue((Param)i);
			if (*(a->params[i]) != value) {
				*(a->params[i]) = value;
			}
		}
	}
}

//////////////////// LADSPA port setup /////////////////////////////////////////

static void connect_port (LADSPA_Handle instance, unsigned long port, LADSPA_Data * data)
{
    amsynth_wrapper * a = (amsynth_wrapper *) instance;
    switch (port)
    {
    case 0: a->out_l = data; break;
    case 1: a->out_r = data; break;
    default:
		if ((port - 2) < kAmsynthParameterCount) { a->params[port-2] = data; }
		break;
    }
}

//////////////////// Audio callback ////////////////////////////////////////////

static void run_synth (LADSPA_Handle instance, unsigned long sample_count, snd_seq_event_t *events, unsigned long event_count)
{
	amsynth_wrapper * a = (amsynth_wrapper *) instance;

	memset(a->midi_buffer, 0, MIDI_BUFFER_SIZE);
	unsigned char *midi_buffer_ptr = a->midi_buffer;

#define push_midi_ev3(__status__, __byte1__, __byte2__) do { \
	midi_buffer_ptr[0] = __status__; \
	midi_buffer_ptr[1] = __byte1__; \
	midi_buffer_ptr[2] = __byte2__; \
	midi_events.push_back((amsynth_midi_event_t){ e->time.tick, 3, midi_buffer_ptr }); \
	midi_buffer_ptr += 3; } while (0)

	std::vector<amsynth_midi_event_t> midi_events;
	for (snd_seq_event_t *e = events; e < events + event_count; e++) {
		switch (e->type) {
		case SND_SEQ_EVENT_NOTEON:
			push_midi_ev3(MIDI_STATUS_NOTE_ON, e->data.note.note, e->data.note.velocity);
			break;
		case SND_SEQ_EVENT_NOTEOFF:
			push_midi_ev3(MIDI_STATUS_NOTE_OFF, e->data.note.note, e->data.note.velocity);
			break;
		case SND_SEQ_EVENT_CONTROLLER:
			if (e->data.control.param < 128 && e->data.control.value < 128) {
				push_midi_ev3(MIDI_STATUS_CONTROLLER, e->data.control.param, e->data.control.value);
			}
			break;
		case SND_SEQ_EVENT_PITCHBEND: {
			unsigned int data = e->data.control.value + 8192;
			unsigned char data1 = (data & 0x7f);
			unsigned char data2 = (data >> 7 & 0x7f);
			push_midi_ev3(MIDI_STATUS_PITCH_WHEEL, data1, data2);
			break;
		}
		case SND_SEQ_EVENT_PGMCHANGE:
			select_program(instance, 0, e->data.control.value);
			break;
		default:
			break;
		}
	}

	for (unsigned i = (Param)0; i < kAmsynthParameterCount; i++) {
		const LADSPA_Data host_value = *(a->params[i]);
		if (a->synth->getParameterValue((Param)i) != host_value) {
			a->synth->setParameterValue((Param)i, host_value);
		}
	}

	std::vector<amsynth_midi_cc_t> midi_out;
	a->synth->process(sample_count, midi_events, midi_out, a->out_l, a->out_r);
}

// renoise ignores DSSI plugins that don't implement run

static void run (LADSPA_Handle instance, unsigned long sample_count)
{
    run_synth (instance, sample_count, nullptr, 0);
}

////////////////////////////////////////////////////////////////////////////////

/*
 * Magic routines called by the system upon opening and closing libraries..
 * http://www.tldp.org/HOWTO/Program-Library-HOWTO/miscellaneous.html#INIT-AND-CLEANUP
 */

void __attribute__ ((constructor)) my_init ()
{
	s_presetController = new PresetController;
    const char **port_names;
    LADSPA_PortDescriptor *port_descriptors;
    LADSPA_PortRangeHint *port_range_hints;

	/* LADSPA descriptor */
    s_ladspaDescriptor = (LADSPA_Descriptor *) calloc (1, sizeof (LADSPA_Descriptor));
    if (s_ladspaDescriptor)
	{
		s_ladspaDescriptor->UniqueID = 23;
		s_ladspaDescriptor->Label = "amsynth";
		s_ladspaDescriptor->Properties = LADSPA_PROPERTY_REALTIME | LADSPA_PROPERTY_HARD_RT_CAPABLE;
		s_ladspaDescriptor->Name = "amsynth DSSI plugin";
		s_ladspaDescriptor->Maker = "Nick Dowell <nick@nickdowell.com>";
		s_ladspaDescriptor->Copyright = "(c) 2005";

		//
		// set up ladspa 'Ports' - used to perform audio and parameter communication...
		//
		
		port_descriptors = (LADSPA_PortDescriptor *) calloc (kAmsynthParameterCount+2, sizeof (LADSPA_PortDescriptor));
		port_range_hints = (LADSPA_PortRangeHint *) calloc (kAmsynthParameterCount+2, sizeof (LADSPA_PortRangeHint));
		port_names = (const char **) calloc (kAmsynthParameterCount+2, sizeof (char *));
		
		// we need ports to transmit the audio data...
		port_descriptors[0] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		port_range_hints[0].HintDescriptor = 0;
		port_names[0] = "OutL";

		port_descriptors[1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		port_range_hints[1].HintDescriptor = 0;
		port_names[1] = "OutR";

		Preset amsynth_preset;
		for (unsigned i=0; i<kAmsynthParameterCount; i++)
		{
			const Parameter &parameter = amsynth_preset.getParameter(i);
			const int numSteps = parameter.getSteps();
			port_descriptors[i+2] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
			port_range_hints[i+2].LowerBound = parameter.getMin();
			port_range_hints[i+2].UpperBound = parameter.getMax();
			port_range_hints[i+2].HintDescriptor = LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE
				| ((numSteps == 2) ? LADSPA_HINT_TOGGLED : 0)
				| ((numSteps  > 2) ? LADSPA_HINT_INTEGER : 0);
				
			// Try to map onto LADSPA's insane take on 'default' values
			const float def = parameter.getValue();
			const float min = parameter.getMin();
			const float max = parameter.getMax();
			const float med = (parameter.getMin() + parameter.getMax()) / 2.0;
			//
			if (def == 0)			port_range_hints[i+2].HintDescriptor |= LADSPA_HINT_DEFAULT_0;
			else if (def == 1)		port_range_hints[i+2].HintDescriptor |= LADSPA_HINT_DEFAULT_1;
			else if (def == 100)	port_range_hints[i+2].HintDescriptor |= LADSPA_HINT_DEFAULT_100;
			else if (def == 440)	port_range_hints[i+2].HintDescriptor |= LADSPA_HINT_DEFAULT_440;
			else if (def == min)	port_range_hints[i+2].HintDescriptor |= LADSPA_HINT_DEFAULT_MINIMUM;
			else if (def == max)	port_range_hints[i+2].HintDescriptor |= LADSPA_HINT_DEFAULT_MAXIMUM;
			else if (def  < med)	port_range_hints[i+2].HintDescriptor |= LADSPA_HINT_DEFAULT_LOW;
			else if (def == med)	port_range_hints[i+2].HintDescriptor |= LADSPA_HINT_DEFAULT_MIDDLE;
			else if (def  > med)	port_range_hints[i+2].HintDescriptor |= LADSPA_HINT_DEFAULT_HIGH;
			
			port_names[i+2] = parameter_name_from_index(i); // returns a pointer to a static string
			TRACE_ARGS("port hints for %32s : %x", parameter_name_from_index(i), port_range_hints[i+2]);
		}
	
		s_ladspaDescriptor->PortDescriptors = port_descriptors;
		s_ladspaDescriptor->PortRangeHints  = port_range_hints;
		s_ladspaDescriptor->PortNames       = port_names;
		s_ladspaDescriptor->PortCount       = kAmsynthParameterCount+2;
	
	
		s_ladspaDescriptor->instantiate = instantiate;
		s_ladspaDescriptor->cleanup = cleanup;

		s_ladspaDescriptor->activate = nullptr;
		s_ladspaDescriptor->deactivate = nullptr;

		s_ladspaDescriptor->connect_port = connect_port;
		s_ladspaDescriptor->run = run;
		s_ladspaDescriptor->run_adding = nullptr;
		s_ladspaDescriptor->set_run_adding_gain = nullptr;
    }

	/* DSSI descriptor */
    s_dssiDescriptor = (DSSI_Descriptor *) malloc (sizeof (DSSI_Descriptor));
    if (s_dssiDescriptor)
	{
		s_dssiDescriptor->DSSI_API_Version				= 1;
		s_dssiDescriptor->LADSPA_Plugin				= s_ladspaDescriptor;
		s_dssiDescriptor->configure					= configure;
		s_dssiDescriptor->get_program 					= get_program;
		s_dssiDescriptor->get_midi_controller_for_port	= nullptr;
		s_dssiDescriptor->select_program 				= select_program;
		s_dssiDescriptor->run_synth 					= run_synth;
		s_dssiDescriptor->run_synth_adding 				= nullptr;
		s_dssiDescriptor->run_multiple_synths 			= nullptr;
		s_dssiDescriptor->run_multiple_synths_adding	= nullptr;
    }
}

void __attribute__ ((destructor)) my_fini ()
{
    if (s_ladspaDescriptor)
	{
		free ((LADSPA_PortDescriptor *) s_ladspaDescriptor->PortDescriptors);
		free ((char **) s_ladspaDescriptor->PortNames);
		free ((LADSPA_PortRangeHint *) s_ladspaDescriptor->PortRangeHints);
		free (s_ladspaDescriptor);
    }
    if (s_dssiDescriptor)
	{
		free (s_dssiDescriptor);
    }
	if (s_presetController)
	{
		delete s_presetController;
	}

}
