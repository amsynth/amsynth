/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#include "MidiController.h"
#include "VoiceAllocationUnit.h"
#include "AudioOutput.h"
#include "JackOutput.h"
#include "GUI/GUI.h"
#include "GUI/ParameterView.h"
#include "Config.h"
#include "../config.h"

#include <gtkmm/main.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>

using namespace std;

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



int the_pipe[2];

void sched_realtime()
{
	struct sched_param sched;

	sched.sched_priority = 50;
	int foo = sched_setscheduler(0, SCHED_FIFO, &sched);
	sched_getparam(0, &sched);

	if (foo) {
		config.realtime = 0;
		/*
		cout << endl << "		 - - WARNING - -" << endl
		<< "    amSynth could not set realtime priority." << endl
		<< "You may experience audio buffer underruns "
		<< "resulting in 'clicks' in the audio." << endl
		<< "This is most likely because the program is not SUID root." << endl
		<< "Please read the documentation for information on how to "
		<< "remedy this." << endl << endl;
		*/
	}
	else {
		config.realtime = 1;
#ifdef _DEBUG
		cout << "main: scheduling priority is " << sched.sched_priority << endl;
#endif
	}
}

void
pipe_event( void *arg, int foo, GdkInputCondition ic )
{
	gui->serve_request();
}

int main( int argc, char *argv[] )
{
	std::cout << 
"amSynth " VERSION "\n\
Copyright 2001-2004 Nick Dowell and others.\n\
amSynth comes with ABSOLUTELY NO WARRANTY\n\
This is free software, and you are welcome to redistribute it\n\
under certain conditions; see the file COPYING for details\n";

	if( pipe( the_pipe ) ) cout << "pipe() error\n";
	bool jack = false;
	
	int opt;
	while( (opt=getopt(argc, argv, "vhstdm:c:a:r:p:b:"))!= -1 ) {
		switch(opt) {
			case 'v':
				cout << "amSynth " << VERSION << " -- compiled "
					<< __DATE__ << " " << __TIME__ << endl;
				return 0;
			case 'h':
				cout << help_text; 
				return 0;
			default:
				break;
		}
	}
	
	
	// setup the configuration
	config.Defaults ();
	config.load ();
	config.ParseCOpts (argc, argv);
	
	if (config.debug_drivers)
		cout << "\n*** CONFIGURATION:\n"
				<< "MIDI:- driver:" << config.midi_driver 
				<< " channel:" << config.midi_channel << endl 
				<< "AUDIO:- driver:" << config.audio_driver 
				<< " sample rate:" << config.sample_rate << endl;

	string amsynth_bank_file = config.current_bank_file;

	presetController = new PresetController();
	
	
	//
	// initialise audio
	//
	if (config.debug_drivers) std::cerr << "\n\n*** INITIALISING AUDIO ENGINE...\n";
	
	if (config.audio_driver=="jack"||config.audio_driver=="JACK")
	{
		jack = 1;
		out = new JackOutput();
		if (((JackOutput*)out)->init (config)!=0)
		{
			std::cerr << ((JackOutput*)out)->get_error_msg() << "\n";
			std::cerr << "** failed to initialise JACK... aborting :'( **\n";
			exit (10);
		}
	}
	else if (config.audio_driver=="auto"||config.audio_driver=="AUTO")
	{
		jack = 1;
		out = new JackOutput();
		if (((JackOutput*) out)->init (config) != 0)
		{
			delete out;
			jack = 0;
			out = new AudioOutput();
		}
	}
	else
	{
		out = new AudioOutput();
	}
	if ((!jack) && (out->init (config) != 0))
	{
		std::cerr << "failed to open any audio device\n\n";
		exit (-1);
	}
	
	//~ out = new AudioOutput ();
	//~ int res = out->init (config);
	//~ if (0 != res)
	//~ {
		//~ std::cerr << "AudioOutput::init() failed with code %d" << res;
	//~ }
	
	vau = new VoiceAllocationUnit;
	vau->SetSampleRate (config.sample_rate);
	vau->SetMaxVoices (config.polyphony);
	out->setInput( vau );
	
	presetController->loadPresets(config.current_bank_file.c_str());
	
	out->Start ();
	
	if (config.debug_drivers) std::cerr << "*** DONE :)\n";
	
	
	//
	// init midi
	//
	if (config.debug_drivers) std::cerr << "\n\n*** INITIALISING MIDI ENGINE...\n";
	
	config.alsa_seq_client_name = out->getTitle();

	midi_controller = new MidiController( config );
	if (midi_controller->init () != 0)
	{
		std::cerr << "failed to open any midi device\n\n";
		exit (-1);
	}
	
	midi_controller->Run ();

	if (config.debug_drivers) std::cerr << "*** DONE :)\n\n";
  
	// need to drop our suid-root permissions :-
	// GTK will not work SUID for security reasons..
	setreuid( getuid(), getuid() );
	setregid( getgid(), getgid() );
	
	midi_controller->setVAU( *vau );
	midi_controller->setPresetController( *presetController );
  
	presetController->getCurrentPreset().AddListenerToAll (vau);

	Gtk::Main kit( &argc, &argv ); // this can be called SUID
	
	// make GDK loop read events from the pipe
	gdk_input_add( the_pipe[0], GDK_INPUT_READ, &pipe_event, (void*)NULL );

	// give audio/midi threads time to start up first..
	if (jack) sleep (1);

	// this can be called SUID:
	gui = new GUI (config, *midi_controller, *vau, the_pipe, out, out->getTitle());
	gui->setPresetController ( *presetController );
	gui->init();
	
	// cannot be called SUID:
	kit.run();
	

#ifdef _DEBUG
	cout << "main() : GUI was terminated, shutting down cleanly.." << endl;
#endif
	
	/*
	 * code to shut down cleanly..
	 */

	presetController->savePresets(config.current_bank_file.c_str ());
	midi_controller->saveConfig();
	
	out->Stop ();
#ifdef _DEBUG
	cout << "joined audioThread" << endl;
#endif		
	
	if (config.xruns) std::cerr << config.xruns << " audio buffer underruns occurred\n";
	
	midi_controller->Stop ();

	delete presetController;
	delete midi_controller;
	delete vau;
	delete out;
	return 0;
}
