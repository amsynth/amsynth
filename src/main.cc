/*
 *  main.cc
 *
 *  Copyright (c) 2001-2012 Nick Dowell
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

#include "main.h"

#include "../config.h"
#include "AudioOutput.h"
#include "Config.h"
#include "drivers/ALSAMidiDriver.h"
#include "drivers/OSSMidiDriver.h"
#include "Effects/denormals.h"
#include "GUI/gui_main.h"
#include "JackOutput.h"
#include "lash.h"
#include "midi.h"
#include "MidiController.h"
#include "VoiceAllocationUnit.h"

#if __APPLE__
#include "drivers/CoreAudio.h"
#endif

#include <iostream>
#include <fstream>
#include <getopt.h>
#include <unistd.h>
#include <string>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

#ifdef _DEBUG
#define DEBUGMSG( ... ) fprintf( stderr, __VA_ARGS__ )
#else
#define DEBUGMSG( ... )
#endif



string help_text =
"usage: " PACKAGE " [options]\n\
\n\
Any options given here override those in the config file ($HOME/.amSynthrc)\n\
\n\
OPTIONS:\n\
\n\
	-h          show this usage message\n\
	-v          show version information\n\
	-d          show some debugging output\n\
\n\
	-b <filename>   use <filename> as the bank to store presets\n\
\n\
	-a <string> set the sound output driver to use [alsa/oss/auto(default)]\n\
	-r <int>    set the sampling rate to use\n\
	-m <string>	set the midi driver to use [alsa/oss/auto(default)]\n\
	-c <int>    set the midi channel to respond to (default=all)\n\
	-p <int>    set the polyphony (maximum active voices)\n\
\n\
	-n <name>   specify the JACK client name to use\n\
	--jack_autoconnect=<true|false>\n\
\n";

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

const char *build_path(const char *path, const char *suffix)
{
	char *result = NULL;
	asprintf(&result, "%s/%s", path, suffix);
	return result;
}

void install_default_files_if_reqd()
{
	const char * factory_config = build_path (PKGDATADIR, "rc");
	const char * factory_bank = build_path (PKGDATADIR, "banks/amsynth_factory.bank");

	const char * user_config = build_path (getenv ("HOME"), ".amSynthrc");
	const char * user_bank = build_path (getenv ("HOME"), ".amSynth.presets");
	
	struct stat st;
	
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

	free((void *)factory_config);
	free((void *)factory_bank);
	
	free((void *)user_config);
	free((void *)user_bank);
}

////////////////////////////////////////////////////////////////////////////////

void ptest ();

static MidiDriver *midiDriver;
static MidiController *midi_controller;
static PresetController *presetController;
static VoiceAllocationUnit *voiceAllocationUnit;
static void amsynth_audio_callback(float *buffer_l, float *buffer_r, unsigned num_frames, int stride, amsynth_midi_event_t *events, unsigned event_count);
static unsigned char *midiBuffer;
static const size_t midiBufferSize = 4096;

////////////////////////////////////////////////////////////////////////////////

GenericOutput * open_audio()
{	
#if	__APPLE__

	if (config.audio_driver == "jack" ||
		config.audio_driver == "JACK" ){
		JackOutput *jack = new JackOutput();
		if (jack->init(config) != 0) {
			delete jack;
			return NULL;
		}
		jack->setMidiHandler(midi_controller);
		return jack;
	}

	if (config.audio_driver == "coreaudio" ||
		config.audio_driver == "auto")
		return CreateCoreAudioOutput();

	return NULL;
	
#else

	if (config.audio_driver == "jack" ||
		config.audio_driver == "auto")
	{
		JackOutput *jack = new JackOutput();
		if (jack->init(config) == 0)
		{
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

static MidiDriver *opened_midi_driver(MidiDriver *driver)
{
	if (driver->open(config) != 0) {
		delete driver;
		return NULL;
	}
	return driver;
}

static void open_midi()
{
	if (config.midi_driver == "alsa" || config.midi_driver == "ALSA") {
		if (!(midiDriver = opened_midi_driver(CreateAlsaMidiDriver()))) {
			std::cerr << "error: could not open ALSA MIDI interface";
		}
		return;
	}

	if (config.midi_driver == "oss" || config.midi_driver == "OSS") {
		if (!(midiDriver = opened_midi_driver(CreateOSSMidiDriver()))) {
			std::cerr << "error: could not open OSS MIDI interface";
		}
		return;
	}

	if (config.midi_driver == "auto") {
		if (!(midiDriver = opened_midi_driver(CreateAlsaMidiDriver()))) {
			if (!(midiDriver = opened_midi_driver(CreateOSSMidiDriver()))) {
				std::cerr << "error: could not open any MIDI interface";
			}
		}
		return;
	}
}

void fatal_error(const std::string & msg)
{
	std::cerr << msg << "\n";
	ShowModalErrorMessage(msg);
	exit(1);
}

unsigned amsynth_timer_callback();

static int signal_received = 0;

static void signal_handler(int signal)
{
	signal_received = signal;
}

int main( int argc, char *argv[] )
{
#ifdef ENABLE_REALTIME
	sched_realtime();
#endif

	srand(time(NULL));
	
	disable_denormals();

	// need to drop our suid-root permissions :-
	// GTK will not work SUID for security reasons..
	setreuid( getuid(), getuid() );
	setregid( getgid(), getgid() );	

	bool no_gui = (getenv("AMSYNTH_NO_GUI") != NULL);

	if (!no_gui)
		gui_kit_init(argc, argv);
	
	int initial_preset_no = 0;

	// needs to be called before our own command line parsing code
	amsynth_lash_process_args(&argc, &argv);
	
	static struct option longopts[] = {
		{ "jack_autoconnect", optional_argument, NULL, 0 },
		{ 0 }
	};
	
	int opt = -1, longindex = -1;
	while ((opt = getopt_long(argc, argv, "vhstdzxm:c:a:r:p:b:U:P:n:", longopts, &longindex)) != -1) {
		switch (opt) {
			case 'v':
				cout << PACKAGE_STRING << " -- compiled " << __DATE__ << " " << __TIME__ << endl;
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
			case 'x':
				no_gui = true;
				break;
			case 'm':
				config.midi_driver = optarg;
				break;
			case 'b':
				config.current_bank_file = optarg;
				break;
			case 'c':
				config.midi_channel = atoi( optarg );
				break;
			case 'a':
				config.audio_driver = optarg;
				break;
			case 'd':
				config.debug_drivers = 1;
				break;
			case 'r':
				config.sample_rate = atoi( optarg );
				break;
			case 'p':
				config.polyphony = atoi( optarg );
				break;
			case 'U':
				config.jack_session_uuid = optarg;
				break;
			case 'n':
				config.jack_client_name_preference = optarg;
				break;
			case 0:
				if (strcmp(longopts[longindex].name, "jack_autoconnect") == 0) {
					JackOutput::autoconnect = !optarg || (strcmp(optarg, "true") == 0);
				}
				break;
			default:
				break;
		}
	}

	// all config files should eventually be migrated to the ~./amsynth directory
	mkdir ((std::string(getenv("HOME")) + std::string("/.amsynth")).c_str(), 0000755);
	mkdir ((std::string(getenv("HOME")) + std::string("/.amsynth") + std::string("/banks")).c_str(), 0000755);

	install_default_files_if_reqd();

	// setup the configuration
	config.Defaults ();
	config.load ();
	
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
	
	presetController = new PresetController();
	
	midi_controller = new MidiController( config );

	GenericOutput *out = open_audio();
	if (!out)
		fatal_error("Fatal Error: open_audio() returned NULL.\n"
					"config.audio_driver = " + config.audio_driver);

	// errors now detected & reported in the GUI
	out->init(config);

	voiceAllocationUnit = new VoiceAllocationUnit;
	voiceAllocationUnit->SetSampleRate (config.sample_rate);
	voiceAllocationUnit->SetMaxVoices (config.polyphony);
	voiceAllocationUnit->setPitchBendRangeSemitones (config.pitch_bend_range);
	out->setAudioCallback (&amsynth_audio_callback);

	amsynth_load_bank(config.current_bank_file.c_str());
	amsynth_set_preset_number(initial_preset_no);
	
	// errors now detected & reported in the GUI
	out->Start();
	
	open_midi();
	midiBuffer = (unsigned char *)malloc(midiBufferSize);

	midi_controller->setMidiDriver(midiDriver);
	midi_controller->SetMidiEventHandler(voiceAllocationUnit);
	midi_controller->setPresetController( *presetController );

	presetController->getCurrentPreset().AddListenerToAll (voiceAllocationUnit);

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

	if (!no_gui) {
		gui_init(config, *midi_controller, *voiceAllocationUnit, *presetController, out);
		gui_kit_run(&amsynth_timer_callback);
		gui_dealloc();
	} else {
		printf("amsynth running in headless mode, press ctrl-c to exit\n");
		signal(SIGINT, &signal_handler);
		while (!signal_received)
			sleep(2); // delivery of a signal will wake us early
		printf("shutting down...\n");
	}

	out->Stop ();

	if (config.xruns) std::cerr << config.xruns << " audio buffer underruns occurred\n";

	delete presetController;
	delete midi_controller;
	delete voiceAllocationUnit;
	delete out;
	return 0;
}

unsigned
amsynth_timer_callback()
{
	amsynth_lash_poll_events();
	midi_controller->timer_callback();
	return 1;
}

void
amsynth_audio_callback(float *buffer_l, float *buffer_r, unsigned num_frames, int stride, amsynth_midi_event_t *events, unsigned event_count)
{
	while (midiDriver) {
		memset(midiBuffer, 0, midiBufferSize);
		int bytes_read = midiDriver->read(midiBuffer, midiBufferSize);
		if (midi_controller && bytes_read > 0) {
			midi_controller->HandleMidiData(midiBuffer, bytes_read);
		} else {
			break;
		}
	}

	if (midi_controller)
		midi_controller->send_changes();

	if (voiceAllocationUnit) {
		std::vector<NoteEvent> note_events;
		for (unsigned i=0; i<event_count; i++) {
			if (events[i].length == 3) {
				switch (events[i].buffer[0] & 0xF0) {
				case MIDI_STATUS_NOTE_OFF:
					note_events.push_back(NoteEvent(events[i].offset_frames, false, events[i].buffer[1], events[i].buffer[2] / 127.0));
					break;
				case MIDI_STATUS_NOTE_ON:
					note_events.push_back(NoteEvent(events[i].offset_frames, true, events[i].buffer[1], events[i].buffer[2] / 127.0));
					break;
				default:
					if (midi_controller)
						midi_controller->HandleMidiData(events[i].buffer, events[i].length);
					break;
				}
			} else {
				if (midi_controller)
					midi_controller->HandleMidiData(events[i].buffer, events[i].length);
			}
		}
		voiceAllocationUnit->Process(num_frames, note_events, buffer_l, buffer_r, stride);
	}
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

	VoiceAllocationUnit *voiceAllocationUnit = new VoiceAllocationUnit;
	voiceAllocationUnit->SetSampleRate (kTestSampleRate);
	
	// trigger off some notes for amSynth to render.
	for (int v=0; v<kNumVoices; v++) voiceAllocationUnit->HandleMidiNoteOn (60+v, 127);
	
	struct rusage usage_before; 
	getrusage (RUSAGE_SELF, &usage_before);
	
	long total_samples = kTestSampleRate * kTimeSeconds;
	long total_calls = total_samples / kTestBufSize;
	long remain_samples = total_samples % kTestBufSize;
	for (int i=0; i<total_calls; i++) voiceAllocationUnit->Process (buffer, buffer, kTestBufSize);
	voiceAllocationUnit->Process (buffer, buffer, remain_samples);

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
	delete voiceAllocationUnit;
}

