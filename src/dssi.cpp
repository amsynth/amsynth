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

#include <dssi.h>
#include <ladspa.h>

#include "Preset.h"

static LADSPA_Descriptor *	s_ladspaDescriptor = NULL;
static DSSI_Descriptor *	s_dssiDescriptor = NULL;


//
// simple dummy synth, to demontrate wrapping.
//

Preset s_preset;

class DssiSynth
{
public:
	DssiSynth () { fprintf (stderr, "DssiSynth::DssiSynth ()\n"); }
	~DssiSynth () { fprintf (stderr, "DssiSynth::~DssiSynth ()\n"); }
	void	SetSampleRate	(unsigned rate) { fprintf (stderr, "DssiSynth::SetSampleRate (%d)\n", rate); }
	void	Process	(	unsigned long sample_count, 
						snd_seq_event_t *events, unsigned long event_count);
	float *bufOut;
};

void
DssiSynth::Process (	unsigned long sample_count, 
						snd_seq_event_t *events, unsigned long event_count)
{
    float *output = bufOut;
   	while (sample_count--)
	{
		*output++ = (float) rand () / (float) RAND_MAX;
    }	
}


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








static void connectPortTS(LADSPA_Handle instance, unsigned long port,
			  LADSPA_Data * data)
{
    DssiSynth *self = (DssiSynth *) instance;
    switch (port)
	{
    case 0:	self->bufOut = data; break;
//    case TS_FREQ:	plugin->freq = data; break;
//    case TS_VOLUME:	plugin->vol = data;	break;
    }
}


/****************************************************
 *
 *	Constructor & Destructor (equivalents)
 *
 ****************************************************/

static LADSPA_Handle instantiateTS(const LADSPA_Descriptor * descriptor,
				   unsigned long s_rate)
{
	DssiSynth *self = new DssiSynth;
	self->SetSampleRate (s_rate);
    return (LADSPA_Handle) self;
}

static void cleanupTS(LADSPA_Handle instance)
{
	DssiSynth *self = (DssiSynth *) instance;
    delete self;
}

/************************************
 *
 *	activate / deactivate
 *
 ************************************/

static void activateTS(LADSPA_Handle instance)
{
}

/*********************************
 *
 *	Audio callback
 *
 *********************************/

static void runTS(	LADSPA_Handle instance, unsigned long sample_count,
					snd_seq_event_t *events, unsigned long event_count)
{
	// do some processing here!
	DssiSynth *self = (DssiSynth*) instance;
	self->Process (sample_count, events, event_count);
}


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
    s_ladspaDescriptor =	(LADSPA_Descriptor *) malloc (sizeof (LADSPA_Descriptor));
    if (s_ladspaDescriptor)
	{
		s_ladspaDescriptor->UniqueID = 23;
		s_ladspaDescriptor->Label = "TS";
		s_ladspaDescriptor->Properties = 0;
		s_ladspaDescriptor->Name = "amSynth";
		s_ladspaDescriptor->Maker = "Nick Dowell <nick@nickdowell.com>";
		s_ladspaDescriptor->Copyright = "(C) 2005";


		//
		// set up ladspa 'Ports' - used to perform audio and parameter communication...
		//
		const unsigned numParams = s_preset.ParameterCount();
		//unsigned numParams = 0;
		
		port_descriptors = new LADSPA_PortDescriptor [numParams+1];
		port_names = new const char * [numParams+1];
		port_range_hints = new LADSPA_PortRangeHint [numParams+1];
		
		// we need a port to transmit the audio data...
		port_descriptors[0] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		port_names[0] = "Output";
		port_range_hints[0].HintDescriptor = 0;

		for (unsigned i=1; i<numParams; i++)
		{
			port_descriptors[i] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
			port_names[i] = s_preset.getParameter(i).getName().c_str();
			port_range_hints[i].HintDescriptor = LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
			port_range_hints[i].LowerBound = s_preset.getParameter(i).getMin();
			port_range_hints[i].UpperBound = s_preset.getParameter(i).getMax();
		}
	
		s_ladspaDescriptor->PortDescriptors = port_descriptors;
		s_ladspaDescriptor->PortRangeHints  = port_range_hints;
		s_ladspaDescriptor->PortNames       = port_names;
		s_ladspaDescriptor->PortCount       = numParams + 1;
	
	
	
		s_ladspaDescriptor->instantiate = instantiateTS;
		s_ladspaDescriptor->cleanup = cleanupTS;

		s_ladspaDescriptor->activate = activateTS;
		s_ladspaDescriptor->deactivate = NULL;

		s_ladspaDescriptor->connect_port = connectPortTS;
		s_ladspaDescriptor->run = NULL;
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
		s_dssiDescriptor->get_program 					= NULL;
		s_dssiDescriptor->get_midi_controller_for_port	= NULL;
		s_dssiDescriptor->select_program 				= NULL;
		s_dssiDescriptor->run_synth 					= runTS;
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
