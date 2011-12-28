/*
 * dssi.c
 * amSynth
 *
 * DSSI wrapper around amsynth
 * Copyright 2005 Nick Dowell
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>

#include <dssi.h>
#include <ladspa.h>

#include "controls.h"
#include "PresetController.h"
#include "VoiceAllocationUnit.h"

#ifdef DEBUG
#define TRACE( msg ) fprintf (stderr, "[amsynth-dssi] %s(): " msg "\n", __func__)
#define TRACE_ARGS( fmt, ... ) fprintf (stderr, "[amsynth-dssi] %s(): " fmt "\n", __func__, __VA_ARGS__)
#else
#define TRACE( msg ) (void)0
#define TRACE_ARGS( ... ) (void)0
#endif

static LADSPA_Descriptor *	s_ladspaDescriptor = NULL;
static DSSI_Descriptor *	s_dssiDescriptor   = NULL;


typedef struct _amsynth_wrapper {
	VoiceAllocationUnit * vau;
	PresetController *    bank;
	LADSPA_Data *         out_l;
	LADSPA_Data *         out_r;
	LADSPA_Data **        params;
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
const DSSI_Descriptor *dssi_descriptor (unsigned long index)
{
    switch (index)
    {
    case 0: return s_dssiDescriptor;
    default: return NULL;
    }
}




//////////////////// Constructor & Destructor (equivalents) ////////////////////

static LADSPA_Handle instantiate (const LADSPA_Descriptor * descriptor, unsigned long s_rate)
{
	TRACE();
    Config config;
    config.Defaults();
    Preset amsynth_preset;
    amsynth_wrapper * a = new amsynth_wrapper;
    a->vau = new VoiceAllocationUnit;
    a->vau->SetSampleRate (s_rate);
    a->bank = new PresetController;
    a->bank->loadPresets(config.current_bank_file.c_str());
    a->bank->selectPreset(0);
    a->bank->getCurrentPreset().AddListenerToAll (a->vau);
    a->params = (LADSPA_Data **) calloc (kControls_End, sizeof (LADSPA_Data *));
    return (LADSPA_Handle) a;
}

static void cleanup (LADSPA_Handle instance)
{
	TRACE();
    amsynth_wrapper * a = (amsynth_wrapper *) instance;
    delete a->vau;
    delete a->bank;
    free (a->params);
    delete a;
}

//////////////////// Program handling //////////////////////////////////////////

static const DSSI_Program_Descriptor *get_program(LADSPA_Handle Instance, unsigned long Index)
{
	amsynth_wrapper * a = (amsynth_wrapper *) Instance;

	static DSSI_Program_Descriptor descriptor;
	memset(&descriptor, 0, sizeof(descriptor));

	if (Index < PresetController::kNumPresets) {
		Preset &preset = a->bank->getPreset(Index);
		descriptor.Bank = 0;
		descriptor.Program = Index;
		descriptor.Name = preset.getName().c_str();
		TRACE_ARGS("%d %d %s", descriptor.Bank, descriptor.Program, descriptor.Name);
		return &descriptor;
	}
	
	return NULL;
}

static void select_program(LADSPA_Handle Instance, unsigned long Bank, unsigned long Index)
{
	amsynth_wrapper * a = (amsynth_wrapper *) Instance;

	TRACE_ARGS("Bank = %d Index = %d", Bank, Index);

	if (Bank == 0 && Index < PresetController::kNumPresets) {
		a->bank->selectPreset(Index);
		// now update DSSI host's view of the parameters
		for (unsigned int i=0; i<kControls_End; i++) {
			*(a->params[i]) = a->bank->getCurrentPreset().getParameter(i).getValue();
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
		if ((port - 2) < kControls_End) { a->params[port-2] = data; }
		break;
    }
}

//////////////////// Audio callback ////////////////////////////////////////////

const float kMidiScaler = (1. / 127.);

static void run_synth (LADSPA_Handle instance, unsigned long sample_count, snd_seq_event_t *events, unsigned long event_count)
{
    amsynth_wrapper * a = (amsynth_wrapper *) instance;

    // process midi events
    for (snd_seq_event_t * e = events; e < events+event_count; e++)
    {
	    switch (e->type)
	    {
		    case SND_SEQ_EVENT_NOTEON:
			    a->vau->HandleMidiNoteOn (e->data.note.note, e->data.note.velocity * kMidiScaler);
			    break;

		    case SND_SEQ_EVENT_NOTEOFF:
			    a->vau->HandleMidiNoteOff (e->data.note.note, e->data.note.velocity * kMidiScaler);
			    break;

		    default:
			    break;
	    }
    }

    // push through changes to parameters
    for (unsigned i=0; i<kControls_End; i++)
    {
		const LADSPA_Data host_value = *(a->params[i]);
	    if (a->bank->getCurrentPreset().getParameter(i).getValue() != host_value)
	    {
			TRACE_ARGS("parameter %32s = %f", a->bank->getCurrentPreset().getParameter(i).getName().c_str(), host_value);
		    a->bank->getCurrentPreset().getParameter(i).setValue(host_value);
	    }
    }

    a->vau->Process ((float *) a->out_l, (float *) a->out_r, sample_count);
}

// renoise ignores DSSI plugins that don't implement run

static void run (LADSPA_Handle instance, unsigned long sample_count)
{
    run_synth (instance, sample_count, NULL, 0);
}

////////////////////////////////////////////////////////////////////////////////

/*
 * Magic routines called by the system upon opening and closing libraries..
 * http://www.tldp.org/HOWTO/Program-Library-HOWTO/miscellaneous.html#INIT-AND-CLEANUP
 */

void __attribute__ ((constructor)) my_init ()
{
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
		
		port_descriptors = (LADSPA_PortDescriptor *) calloc (kControls_End+2, sizeof (LADSPA_PortDescriptor));
		port_range_hints = (LADSPA_PortRangeHint *) calloc (kControls_End+2, sizeof (LADSPA_PortRangeHint));
		port_names = (const char **) calloc (kControls_End+2, sizeof (char *));
		
		// we need ports to transmit the audio data...
		port_descriptors[0] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		port_range_hints[0].HintDescriptor = 0;
		port_names[0] = "OutL";

		port_descriptors[1] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		port_range_hints[1].HintDescriptor = 0;
		port_names[1] = "OutR";

		Preset amsynth_preset;
		for (unsigned i=0; i<kControls_End; i++)
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
			const float rng = parameter.getMax() - parameter.getMin();
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
		s_ladspaDescriptor->PortCount       = kControls_End+2;
	
	
		s_ladspaDescriptor->instantiate = instantiate;
		s_ladspaDescriptor->cleanup = cleanup;

		s_ladspaDescriptor->activate = NULL;
		s_ladspaDescriptor->deactivate = NULL;

		s_ladspaDescriptor->connect_port = connect_port;
		s_ladspaDescriptor->run = run;
		s_ladspaDescriptor->run_adding = NULL;
		s_ladspaDescriptor->set_run_adding_gain = NULL;
    }

	/* DSSI descriptor */
    s_dssiDescriptor = (DSSI_Descriptor *) malloc (sizeof (DSSI_Descriptor));
    if (s_dssiDescriptor)
	{
		s_dssiDescriptor->DSSI_API_Version				= 1;
		s_dssiDescriptor->LADSPA_Plugin				= s_ladspaDescriptor;
		s_dssiDescriptor->configure					= NULL;
		s_dssiDescriptor->get_program 					= get_program;
		s_dssiDescriptor->get_midi_controller_for_port	= NULL;
		s_dssiDescriptor->select_program 				= select_program;
		s_dssiDescriptor->run_synth 					= run_synth;
		s_dssiDescriptor->run_synth_adding 			= NULL;
		s_dssiDescriptor->run_multiple_synths 			= NULL;
		s_dssiDescriptor->run_multiple_synths_adding	= NULL;
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
}
