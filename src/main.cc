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

#include "binreloc.h"

#include <gtkmm/main.h>
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>


using namespace std;


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
-z		run a performance benchmark\n\
-h		show this usage message\n";

Config config;





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


int gdk_input_pipe[2];
void gdk_input_function(gpointer data, gint source, GdkInputCondition condition)
{
	GUI *gui = (GUI *)data;
	gui->serve_request();
}

int fcopy (const char * dest, const char *source)
{
	FILE *in = fopen (source,"r");
	if (in == NULL) {
		fprintf (stderr, "error reading source file %s\n", source);
		return -1;
	}
	FILE *out = fopen (dest,"w");
	if (out == NULL) {
		fprintf (stderr, "error creating destination file %s\n", dest);
		return -1;
	}
	fseek (in, 0, SEEK_END);
	long size = ftell (in);
	rewind (in);
	char * tmp = (char *) malloc (size);
	fread (tmp, 1, size, in);
	fwrite (tmp, 1, size, out);
	free (tmp);
	fclose (in);
	fclose (out);
	return 0;
}

void install_default_files_if_reqd()
{
	BrInitError br_err;
	if (br_init (&br_err) == 0 && br_err != BR_INIT_ERROR_DISABLED) { 
		printf ("Warning: BinReloc failed to initialize (error code %d)\n", br_err); 
		printf ("Will fallback to hardcoded default path.\n"); 
	}
	
	#define DEFAULT_PREFIX "/usr/local"
	char * homedir = getenv ("HOME");
	char * data_dir = br_find_data_dir (DEFAULT_PREFIX"/share");
	char * amsynth_data_dir = br_strcat (data_dir, "/amSynth");
	char * factory_controllers = br_strcat (amsynth_data_dir, "/Controllersrc");
	char * factory_bank = br_strcat (amsynth_data_dir, "/presets");
	char * user_controllers = br_strcat (homedir, "/.amSynthControllersrc");
	char * user_bank = br_strcat (homedir, "/.amSynth.presets");
	
	if (fopen (user_controllers,"r") == NULL)
	{
		printf ("installing default controller map\n");
		fcopy (user_controllers, factory_controllers);
	}
	if (fopen (user_bank,"r") == NULL)
	{
		printf ("installing default sound bank\n");
		fcopy (user_bank, factory_bank);
	}

	free (homedir);
	free (data_dir);
	free (amsynth_data_dir);
	free (factory_controllers);
	free (factory_bank);
	free (user_controllers);
	free (user_bank);
}

void ptest ();

int main( int argc, char *argv[] )
{
	std::cout << 
"amSynth " VERSION "\n\
Copyright 2001-2006 Nick Dowell and others.\n\
amSynth comes with ABSOLUTELY NO WARRANTY\n\
This is free software, and you are welcome to redistribute it\n\
under certain conditions; see the file COPYING for details\n";

	bool jack = false;
	
	int opt;
	while( (opt=getopt(argc, argv, "vhstdzm:c:a:r:p:b:"))!= -1 ) {
		switch(opt) {
			case 'v':
				cout << "amSynth " << VERSION << " -- compiled "
					<< __DATE__ << " " << __TIME__ << endl;
				return 0;
			case 'h':
				cout << help_text; 
				return 0;
			case 'z':
				ptest();
				return 0;
			default:
				break;
		}
	}
	
	install_default_files_if_reqd();

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

	//
	// subsystem initialisation
	//
	
	PresetController *presetController;
	MidiController *midi_controller;
	MidiInterface* midi_interface;
	VoiceAllocationUnit *vau;
	GenericOutput *out;
	GUI *gui;
	
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
	
	midi_interface = new MidiInterface();
	if (midi_interface->open(config) != 0)
	{
		std::cerr << "couldn't to open a midi device\n\n";
		exit (-1);
	}

	midi_controller = new MidiController( config );
	midi_interface->SetMidiStreamReceiver(midi_controller);
	midi_interface->Run();

	if (config.debug_drivers) std::cerr << "*** DONE :)\n\n";
  
	// need to drop our suid-root permissions :-
	// GTK will not work SUID for security reasons..
	setreuid( getuid(), getuid() );
	setregid( getgid(), getgid() );
	
	midi_controller->SetMidiEventHandler(vau);
	midi_controller->setPresetController( *presetController );
  
	presetController->getCurrentPreset().AddListenerToAll (vau);

	Gtk::Main kit( &argc, &argv ); // this can be called SUID

	// give audio/midi threads time to start up first..
	if (jack) sleep (1);

	// this can be called SUID:
	if( pipe( gdk_input_pipe ) ) cout << "pipe() error\n";
	gui = new GUI (config, *midi_controller, *vau, gdk_input_pipe, out, out->getTitle());
	gui->setPresetController ( *presetController );
	gui->init();
	
	// make GDK loop read events from the pipe
	gdk_input_add( gdk_input_pipe[0], GDK_INPUT_READ, &gdk_input_function, gui );
	
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
	config.save();
	
	out->Stop ();
#ifdef _DEBUG
	cout << "joined audioThread" << endl;
#endif		
	
	if (config.xruns) std::cerr << config.xruns << " audio buffer underruns occurred\n";
	
	midi_interface->Stop ();

	delete presetController;
	delete midi_controller;
	delete midi_interface;
	delete vau;
	delete out;
	return 0;
}

///////////////////////////////////////////////////////////////////////////////

void ptest ()
{
	//
	// test parameters
	// 
	const int kTestBufSize = 256;
	const int kTestSampleRate = 44100;
	const int kTimeSeconds = 60;
	const int kNumVoices = 10;

	float *buffer = new float [kTestBufSize];

	VoiceAllocationUnit *vau = new VoiceAllocationUnit;
	vau->SetSampleRate (kTestSampleRate);
	
	// trigger off some notes for amSynth to render.
	for (int v=0; v<kNumVoices; v++) vau->HandleMidiNoteOn (60+v, 127);
	
	struct rusage usage_before; 
	getrusage (RUSAGE_SELF, &usage_before);
	
	long total_samples = kTestSampleRate * kTimeSeconds;
	long total_calls = total_samples / kTestBufSize;
	long remain_samples = total_samples % kTestBufSize;
	for (int i=0; i<total_calls; i++) vau->Process (buffer, buffer, kTestBufSize);
	vau->Process (buffer, buffer, remain_samples);

	struct rusage usage_after; 
	getrusage (RUSAGE_SELF, &usage_after);
	
	unsigned long user_usec = (usage_after.ru_utime.tv_sec*1000000 + usage_after.ru_utime.tv_usec)
							- (usage_before.ru_utime.tv_sec*1000000 + usage_before.ru_utime.tv_usec);
	
	unsigned long syst_usec = (usage_after.ru_stime.tv_sec*1000000 + usage_after.ru_stime.tv_usec)
							- (usage_before.ru_stime.tv_sec*1000000 + usage_before.ru_stime.tv_usec);

	unsigned long usec_audio = kTimeSeconds * kNumVoices * 1000000;
	unsigned long usec_cpu = user_usec + syst_usec;
	
	fprintf (stderr, "user time: %f		system time: %f\n", user_usec/1000000.f, syst_usec/1000000.f);
	fprintf (stderr, "performance index: %f\n", (float) usec_audio / (float) usec_cpu);
	
	delete [] buffer;
	delete vau;
}

