/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "MidiController.h"
#include "VoiceAllocationUnit.h"
#include "AudioOutput.h"
#include "GUI/GUI.h"
#include "GUI/ParameterView.h"
#include "Config.h"
#include <string>

PresetController *presetController;
MidiController *midi_controller;
VoiceAllocationUnit *vau;
AudioOutput *out;
Config config;
GUI *gui;

pthread_t midiThread, guiThread, audioThread;

string help_text =
"usage: amSynth [options]\n\
\n\
any options given here override those in the config file ($HOME/.amSynthrc)\n\
\n\
options:\n\
-m device	set the (OSS) midi device file to use\n\
-c channel	set the midi channel to respond to (default=all)\n\
-d device	set the (OSS) sound output device file to use\n\
-r rate		set the sampling rate to use\n\
-p voices	set the polyphony (maximum active voices)\n\
-s		enables silent mode - no audio ouput\n\
-v		show version.\n\
-h		show this usage message\n";
