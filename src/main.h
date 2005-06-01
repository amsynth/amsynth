/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "MidiController.h"
#include "VoiceAllocationUnit.h"
#include "AudioOutput.h"
#include "JackOutput.h"
#include "GUI/GUI.h"
#include "GUI/ParameterView.h"
#include "Config.h"
#include "base.h"
#include <string>

PresetController *presetController;
MidiController *midi_controller;
VoiceAllocationUnit *vau;
GenericOutput *out;
Config config;
GUI *gui;

string help_text =
"usage: amSynth [options]\n\
\n\
any options given here override those in the config file ($HOME/.amSynthrc)\n\
\n\
options:\n\
-b <filename>	use <filename> as the bank to store presets,\n\
		default = ~/.amSynth.presets\n\
-m device	set the midi driver to use [alsa/oss/auto(default)]\n\
-c channel	set the midi channel to respond to (default=all)\n\
-a device	set the sound output driver to use [alsa/oss/auto(default)]\n\
-r rate		set the sampling rate to use\n\
-p voices	set the polyphony (maximum active voices)\n\
-v		show version.\n\
-d		show some debugging output\n\
-h		show this usage message\n";
