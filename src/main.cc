/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "main.h"
#include "../config.h"

#include <gtk--/main.h>
#include <pthread.h>
#include <iostream>
#include <fstream>
#include <unistd.h>

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


void *midi_thread(void *arg)
{
#ifdef _DEBUG
    cout << "midi_thread() starting" << endl;
#endif
    int foo = (int) arg;
    foo = 0;
	// run midi thread with real-time priority
	/* if we don't do this, the audio thread can lock the system due to its 
	 * priority. if the midi thread has same priority, it can still control 
	 * the audio thread, and the system can return to normal when voices are 
	 * deleted. */
	sched_realtime();
    midi_controller->run();
#ifdef _DEBUG
    cout << "midi_thread() terminated" << endl;
#endif
    pthread_exit(0);
}

void *gui_thread(void *arg)
// it would be nice to have a GUI thread, so it could run without GUI...
{
#ifdef _DEBUG
	cout << "gui_thread() starting" << endl;
#endif
	int foo = (int) arg;
	foo = 0; // not useful - just gets rid of compiler warning
	//kit->run();
#ifdef _DEBUG
	cout << "gui_thread() terminated" << endl;
#endif
	pthread_exit(0);
}


void *audio_thread(void *arg)
{
#ifdef _DEBUG
	cout << "audio_thread() starting" << endl;
#endif
	int foo = (int) arg;
	foo = 0;

	// set realtime priority for this (the audio) thread.
	sched_realtime();
	out->run();
#ifdef _DEBUG
	cout << "audio_thread() terminated" << endl;
#endif
	pthread_exit(0);
}

void
pipe_event( void *arg, int foo, GdkInputCondition ic )
{
	gui->serve_request();
}

int main( int argc, char *argv[] )
{
	std::cout << 
"amSynth 1.0.0\n\
Copyright 2001-2004 Nick Dowell and others.\n\
amSynth comes with ABSOLUTELY NO WARRANTY\n\
This is free software, and you are welcome to redistribute it\n\
under certain conditions; see the file COPYING for details\n";

	if( pipe( the_pipe ) ) cout << "pipe() error\n";
	int jack = 0;
	int enable_audio = 1;
	int enable_gui = 1;
	
	
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
			case 's':
				enable_audio = 0;
				break;
			case 't':
				enable_gui = 0;
				break;				
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
	
	if (enable_audio)
	{
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
			if (((JackOutput*)out)->init (config)!=0)
			{
				jack = 0;
				out = new AudioOutput();
			}
		}
		else
		{
			out = new AudioOutput();
		}

		if (jack==0) if (out->init (config) != 0)
		{
			std::cerr << "failed to open any audio device\n\n";
			exit (-1);
		}
	}
	
	vau = new VoiceAllocationUnit( config ); // were sure of sample_rate now
	if (enable_audio) out->setInput( vau );
	
	presetController->loadPresets(config.current_bank_file.c_str());
	
	int audio_res;
	if( enable_audio )
	{
		if (jack) out->run();
		else audio_res = 
			pthread_create(&audioThread, NULL, audio_thread, NULL);
	}
	
	if (config.debug_drivers) std::cerr << "*** DONE :)\n";
	
	
	//
	// init midi
	//
	if (config.debug_drivers) std::cerr << "\n\n*** INITIALISING MIDI ENGINE...\n";
	
	if (enable_audio)
	{
		config.alsa_seq_client_name = out->getTitle();
		midi_controller = new MidiController( config );
	}
	else
		midi_controller = new MidiController( config );

	if (midi_controller->init () != 0)
	{
		std::cerr << "failed to open any midi device\n\n";
		exit (-1);
	}
	
	int midi_res;
	if (enable_gui) midi_res = 
			pthread_create( &midiThread, NULL, midi_thread, NULL );
	if (config.debug_drivers) std::cerr << "*** DONE :)\n\n";
  
	// need to drop our suid-root permissions :-
	// GTK will not work SUID for security reasons..
	setreuid( getuid(), getuid() );
	setregid( getgid(), getgid() );
	
	midi_controller->setVAU( *vau );
	midi_controller->setPresetController( *presetController );
  
	presetController->getCurrentPreset().AddListenerToAll (vau);

	presetController->selectPreset( 1 );
        presetController->selectPreset( 0 );
	
	if (enable_gui==1)
	{	
	Gtk::Main kit( &argc, &argv ); // this can be called SUID
	
	// make GDK loop read events from the pipe
	gdk_input_add( the_pipe[0], GDK_INPUT_READ, &pipe_event, (void*)NULL );

	// give audio/midi threads time to start up first..
	if (enable_audio && jack) sleep( 1 );

	// this can be called SUID:
	if (enable_audio)
		gui = new GUI( config, *midi_controller, *vau, the_pipe, out, out->getTitle() );
	else
		gui = new GUI( config, *midi_controller, *vau, the_pipe, 0, (const char*)"amSynth (silent)" );
	gui->setPresetController( *presetController );
	gui->init();
	if (config.xfontname!="")
	{
		gui->set_x_font ( config.xfontname.c_str() );
	}
	
	// cannot be called SUID:
	kit.run();
	}
	else midi_controller->run();
	

#ifdef _DEBUG
	cout << "main() : GUI was terminated, shutting down cleanly.." << endl;
#endif
	
	/*
	 * code to shut down cleanly..
	 */

	config.xfontname = gui->get_x_font ();
	config.save ();
		
	presetController->savePresets(config.current_bank_file.c_str ());
	midi_controller->saveConfig();
	
	if(enable_audio)
	{
		out->stop();
		if (!jack) audio_res = pthread_join(audioThread, NULL);
#ifdef _DEBUG
		cout << "joined audioThread" << endl;
#endif		
	}
	
	if (config.xruns) 
		std::cerr << config.xruns << " audio buffer underruns occurred\n";
	
	midi_controller->stop();
	// we probably need to kill the midi thread, as it is always waiting for
	// a new midi event..
	midi_res = pthread_kill( midiThread, 2 );
	midi_res = pthread_join( midiThread, NULL );
#ifdef _DEBUG
	cout << "joined midiThread" << endl;
#endif	
	delete presetController;
	delete midi_controller;
	delete vau;
	delete out;
	return 0;
}
