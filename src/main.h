/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "MidiController.h"
#include "VoiceAllocationUnit.h"
#include "AudioOutput.h"
#include "GUI/GUI.h"
#include "Config.h"
#include <string>

PresetController *presetController;
MidiController *midi_controller;
VoiceAllocationUnit *vau;
AudioOutput *out;
Config config;

pthread_t midiThread, guiThread, audioThread;

string help_text =
"usage: amSynth [-h|-v|-m device|-c channel|-d device|-r list]\n\
\n\
any options given here override those in the config file ($HOME/.amSynthrc)\n\
\n\
-m device	set the midi device file to use\n\
-c channel	set the midi channel to respond to (default=all)\n\
-d device	set the sound output device file to use\n\
-r rate		set the sampling rate to use\n\
-s			enables silent mode - no audio ouput\n\
-v		show version.\n\
-h 		show this usage message\n";
