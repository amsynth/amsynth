/* -*- c-basic-offset: 4 -*-  vi:set ts=8 sts=4 sw=4: */

/* trivial_synth.c

   Disposable Hosted Soft Synth API
   Constructed by Chris Cannam, Steve Harris and Sean Bolton

   This is an example DSSI synth plugin written by Steve Harris.

   This example file is in the public domain.
*/

#include <stdlib.h>
#include <string.h>

#include <math.h>
#include <stdio.h>

#include "dssi.h"
#include "ladspa.h"

#define TS_OUTPUT 0
#define TS_FREQ   1
#define TS_VOLUME 2

#define MIDI_NOTES 128

static LADSPA_Descriptor *tsLDescriptor = NULL;
static DSSI_Descriptor *tsDDescriptor = NULL;


typedef struct {
    unsigned int active;
    float amp;
    double phase;
} note_data;

typedef struct {
    LADSPA_Data *output;
    LADSPA_Data *freq;
    LADSPA_Data *vol;
    note_data data[MIDI_NOTES];
    float omega[MIDI_NOTES];
} TS;



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
    case 0: return tsDDescriptor;
	default: return NULL;
    }
}








static void connectPortTS(LADSPA_Handle instance, unsigned long port,
			  LADSPA_Data * data)
{
    DssiSynth *self = (DssiSynth *) instance;
    switch (port)
	{
    case TS_OUTPUT:	self->bufOut = data; break;
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
    char **port_names;
    LADSPA_PortDescriptor *port_descriptors;
    LADSPA_PortRangeHint *port_range_hints;

	/* LADSPA descriptor */
    tsLDescriptor =	(LADSPA_Descriptor *) malloc (sizeof (LADSPA_Descriptor));
    if (tsLDescriptor)
	{
		tsLDescriptor->UniqueID = 23;
		tsLDescriptor->Label = "TS";
		tsLDescriptor->Properties = 0;
		tsLDescriptor->Name = "amSynth";
		tsLDescriptor->Maker = "Nick Dowell <nick@nickdowell.com>";
		tsLDescriptor->Copyright = "(C) 2005";


		/* LADPSA ports */
		tsLDescriptor->PortCount = 3;		
	
		port_descriptors = (LADSPA_PortDescriptor *) calloc (tsLDescriptor->PortCount, sizeof (LADSPA_PortDescriptor));
		tsLDescriptor->PortDescriptors = (const LADSPA_PortDescriptor *) port_descriptors;
	
		port_range_hints = (LADSPA_PortRangeHint *) calloc (tsLDescriptor->PortCount, sizeof (LADSPA_PortRangeHint));
		tsLDescriptor->PortRangeHints = (const LADSPA_PortRangeHint *) port_range_hints;
	
		port_names = (char **) calloc(tsLDescriptor->PortCount, sizeof(char *));
		tsLDescriptor->PortNames = (const char **) port_names;
	
		/* Parameters for Output */
		port_descriptors[TS_OUTPUT] = LADSPA_PORT_OUTPUT | LADSPA_PORT_AUDIO;
		port_names[TS_OUTPUT] = "Output";
		port_range_hints[TS_OUTPUT].HintDescriptor = 0;
	
		/* Parameters for Freq */
		port_descriptors[TS_FREQ] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		port_names[TS_FREQ] = "Tuning frequency";
		port_range_hints[TS_FREQ].HintDescriptor = LADSPA_HINT_DEFAULT_440 | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
		port_range_hints[TS_FREQ].LowerBound = 420;
		port_range_hints[TS_FREQ].UpperBound = 460;
	
		/* Parameters for Volume */
		port_descriptors[TS_VOLUME] = LADSPA_PORT_INPUT | LADSPA_PORT_CONTROL;
		port_names[TS_VOLUME] = "Volume";
		port_range_hints[TS_VOLUME].HintDescriptor = LADSPA_HINT_DEFAULT_MAXIMUM | LADSPA_HINT_BOUNDED_BELOW | LADSPA_HINT_BOUNDED_ABOVE;
		port_range_hints[TS_VOLUME].LowerBound = 0.0;
		port_range_hints[TS_VOLUME].UpperBound = 1.0;
	
	
	
		tsLDescriptor->instantiate = instantiateTS;
		tsLDescriptor->cleanup = cleanupTS;

		tsLDescriptor->activate = activateTS;
		tsLDescriptor->deactivate = NULL;

		tsLDescriptor->connect_port = connectPortTS;
		tsLDescriptor->run = NULL;
		tsLDescriptor->run_adding = NULL;
		tsLDescriptor->set_run_adding_gain = NULL;
    }

	/* DSSI descriptor */
    tsDDescriptor = (DSSI_Descriptor *) malloc (sizeof (DSSI_Descriptor));
    if (tsDDescriptor)
	{
		tsDDescriptor->DSSI_API_Version				= 1;
		tsDDescriptor->LADSPA_Plugin				= tsLDescriptor;
		tsDDescriptor->configure					= NULL;
		tsDDescriptor->get_program 					= NULL;
		tsDDescriptor->get_midi_controller_for_port	= NULL;
		tsDDescriptor->select_program 				= NULL;
		tsDDescriptor->run_synth 					= runTS;
		tsDDescriptor->run_synth_adding 			= NULL;
		tsDDescriptor->run_multiple_synths 			= NULL;
		tsDDescriptor->run_multiple_synths_adding	= NULL;
    }
}

void __attribute__ ((destructor)) my_fini ()
{
    if (tsLDescriptor)
	{
		free ((LADSPA_PortDescriptor *) tsLDescriptor->PortDescriptors);
		free ((char **) tsLDescriptor->PortNames);
		free ((LADSPA_PortRangeHint *) tsLDescriptor->PortRangeHints);
		free (tsLDescriptor);
    }
    if (tsDDescriptor)
	{
		free (tsDDescriptor);
    }
}
