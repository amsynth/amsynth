/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#include "main.h"
#include "GUI/gui_main.h"
#include "MidiController.h"
#include "VoiceAllocationUnit.h"
#include "AudioOutput.h"
#include "JackOutput.h"
#include "Config.h"
#include "../config.h"
#include "lash.h"

#if __APPLE__
#include "drivers/CoreAudio.h"
#endif

#include "binreloc.h"

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "Effects/denormals.h"

using namespace std;

#ifdef _DEBUG
#define DEBUGMSG( ... ) fprintf( stderr, __VA_ARGS__ )
#else
#define DEBUGMSG( ... )
#endif

#ifndef DEFAULT_PREFIX
#define DEFAULT_PREFIX "/usr/local"
#endif


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

#ifdef ENABLE_REALTIME
void sched_realtime()
{
#ifdef linux
	struct sched_param sched = {0};
	sched.sched_priority = 50;
	int foo = sched_setscheduler(0, SCHED_FIFO, &sched);
	sched_getparam(0, &sched);

	if (foo) {
		DEBUGMSG("Failed to set SCHED_FIFO\n");
		config.realtime = 0;
	}
	else {
		DEBUGMSG("Set SCHED_FIFO\n");
		config.realtime = 1;
	}
#elif defined(__APPLE__)
	// CoreAudio apps don't need realtime priority for decent performance
#else
#warning "sched_realtime not implemented for this OS"
#endif
}
#endif

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
	if (fread(tmp, 1, size, in) && 
		fwrite(tmp, 1, size, out))
		{}
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
	
	char * homedir = getenv ("HOME");
	char * data_dir = br_find_data_dir (DEFAULT_PREFIX "/share");
	char * amsynth_data_dir = br_strcat (data_dir, "/amSynth");
	char * factory_controllers = br_strcat (amsynth_data_dir, "/Controllersrc");
	char * factory_config = br_strcat (amsynth_data_dir, "/rc");
	char * factory_bank = br_strcat (amsynth_data_dir, "/presets");
	char * user_controllers = br_strcat (homedir, "/.amSynthControllersrc");
	char * user_config = br_strcat (homedir, "/.amSynthrc");
	char * user_bank = br_strcat (homedir, "/.amSynth.presets");
	
	struct stat st;
	
	if (stat (user_controllers, &st) == -1)
	{
		printf ("installing default controller map to %s\n", user_controllers);
		fcopy (user_controllers, factory_controllers);
	}
	if (stat (user_config, &st) == -1)
	{
		printf ("installing configuration file to %s\n", user_config);
		fcopy (user_config, factory_config);
	}
	if (stat (user_bank, &st) == -1)
	{
		printf ("installing default sound bank to %s\n", user_bank);
		fcopy (user_bank, factory_bank);
	}

//	free (homedir); // NO!
	free (data_dir);
	free (amsynth_data_dir);
	free (factory_controllers);
	free (factory_bank);
	free (user_controllers);
	free (user_bank);
}

void ptest ();

static MidiController *midi_controller = NULL;
static PresetController *presetController = NULL;

GenericOutput * open_audio()
{	
#if	__APPLE__

	return CreateCoreAudioOutput();
	
#else

	if (config.audio_driver == "jack" ||
		config.audio_driver == "auto")
	{
		JackOutput *jack = new JackOutput();
		if (jack->init(config) == 0)
		{
			if (config.current_midi_driver == "JACK" ||
				config.current_midi_driver == "jack" )
			{
				jack->setMidiHandler(midi_controller);
			}
			return jack;
		}
		else
		{
			std::string jack_error = jack->get_error_msg();
			delete jack;
			
			// we were asked specifically for jack, so don't use anything else
			if (config.audio_driver == "jack") {
				std::cerr << "JACK init failed: " << jack_error << "\n";
				return new NullAudioOutput;
			}
		}
	}
	
	return new AudioOutput();
	
#endif
}

void fatal_error(const std::string & msg)
{
	std::cerr << msg << "\n";
	ShowModalErrorMessage(msg);
	exit(1);
}

unsigned amsynth_timer_callback();

int main( int argc, char *argv[] )
{
#ifdef ENABLE_REALTIME
	sched_realtime();
#endif
	
	disable_denormals();

	// need to drop our suid-root permissions :-
	// GTK will not work SUID for security reasons..
	setreuid( getuid(), getuid() );
	setregid( getgid(), getgid() );	
	
	gui_kit_init(argc, argv);
	
	// will need to change when we reach the year 10000 ;-)
	std::string build_year(__DATE__, sizeof(__DATE__) - 5, 4);
	
	std::cout <<
		"amSynth " VERSION "\n"
		"Copyright 2001-" << build_year << " Nick Dowell and others.\n"
		"amSynth comes with ABSOLUTELY NO WARRANTY\n"
		"This is free software, and you are welcome to redistribute it\n"
		"under certain conditions; see the file COPYING for details\n";

	int initial_preset_no = 0;

	// needs to be called before our own command line parsing code
	amsynth_lash_process_args(&argc, &argv);

	int opt;
	while( (opt=getopt(argc, argv, "vhstdzm:c:a:r:p:b:U:P:"))!= -1 ) {
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
			case 'P':
				initial_preset_no = atoi(optarg);
				break;
			default:
				break;
		}
	}
	
	install_default_files_if_reqd();
	
	char *data_dir = br_find_data_dir (DEFAULT_PREFIX "/share");
	char *amsynth_data_dir = br_strcat (data_dir, "/amSynth");
	setenv ("AMSYNTH_DATA_DIR", amsynth_data_dir, 0); // don't override value passed from command line
	free (amsynth_data_dir);
	free (data_dir);
	
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
	
	MidiInterface *midi_interface = NULL;
	VoiceAllocationUnit *vau = NULL;
	GenericOutput *out = NULL;
	
	presetController = new PresetController();
	
	midi_controller = new MidiController( config );

	out = open_audio();
	if (!out)
		fatal_error("Fatal Error: open_audio() returned NULL.\n"
		            "config.audio_driver = " + config.audio_driver);

	// errors now detected & reported in the GUI
	out->init(config);

	vau = new VoiceAllocationUnit;
	vau->SetSampleRate (config.sample_rate);
	vau->SetMaxVoices (config.polyphony);
	out->setInput( vau );

	amsynth_load_bank(config.current_bank_file.c_str());
	amsynth_set_preset_number(initial_preset_no);
	
	// errors now detected & reported in the GUI
	out->Start();
	
	if (config.debug_drivers) std::cerr << "*** DONE :)\n";

	//
	// init midi
	//
#if __APPLE__
	midi_interface = CreateCoreMidiInterface();
#else
	if (config.debug_drivers) std::cerr << "\n\n*** INITIALISING MIDI ENGINE...\n";
	
	config.alsa_seq_client_name = out->getTitle();
	
	if (config.current_midi_driver.empty())
		midi_interface = new MidiInterface();
#endif
	
	// errors now detected & reported in the GUI
	if (midi_interface) {
		midi_interface->open(config);
		midi_interface->SetMidiStreamReceiver(midi_controller);
		midi_interface->Run();
	}

	if (config.debug_drivers) std::cerr << "*** DONE :)\n\n";
  
	midi_controller->SetMidiEventHandler(vau);
	midi_controller->setPresetController( *presetController );
  
	presetController->getCurrentPreset().AddListenerToAll (vau);

	// prevent lash from spawning a new jack server
	setenv("JACK_NO_START_SERVER", "1", 0);
	
	if (config.alsa_seq_client_id != 0 || !config.jack_client_name.empty())
		// LASH only works with ALSA MIDI and JACK
		amsynth_lash_init();

	if (config.alsa_seq_client_id != 0) // alsa midi is active
		amsynth_lash_set_alsa_client_id(config.alsa_seq_client_id);

	if (!config.jack_client_name.empty())
		amsynth_lash_set_jack_client_name(config.jack_client_name.c_str());

	// give audio/midi threads time to start up first..
	// if (jack) sleep (1);
	
	gui_init(config, *midi_controller, *vau, *presetController, out);
	
	gui_kit_run(&amsynth_timer_callback);

	DEBUGMSG("main() : GUI was terminated, shutting down cleanly..\n");
	
	gui_dealloc();
	
	/*
	 * code to shut down cleanly..
	 */

	midi_controller->saveConfig();
	
	out->Stop ();

	if (config.xruns) std::cerr << config.xruns << " audio buffer underruns occurred\n";
	
	if (midi_interface) {
		midi_interface->Stop ();
		delete midi_interface;
	}

	delete presetController;
	delete midi_controller;
	delete vau;
	delete out;
	return 0;
}

unsigned
amsynth_timer_callback()
{
	amsynth_lash_poll_events();
	return 1;
}

void
amsynth_midi_callback(unsigned /* timestamp */, unsigned num_bytes, unsigned char *midi_data)
{
	if (midi_controller)
		midi_controller->HandleMidiData(midi_data, num_bytes);
}

void
amsynth_save_bank(const char *filename)
{
	presetController->commitPreset();
	presetController->savePresets(filename);
}

void
amsynth_load_bank(const char *filename)
{
	presetController->loadPresets(filename);
	presetController->selectPreset(presetController->getCurrPresetNumber());
}

int
amsynth_get_preset_number()
{
	return presetController->getCurrPresetNumber();
}

void
amsynth_set_preset_number(int preset_no)
{
	presetController->selectPreset(preset_no);
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

