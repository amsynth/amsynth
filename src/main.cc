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
	if( pipe( the_pipe ) ) cout << "pipe() error\n";
	int jack = 0;
	int enable_audio = 1;
	int enable_gui = 1;
	int load_font;
	string xfontname;
	
	// set default parameters
	config.audio_driver = "auto";
	config.midi_driver = "auto";
	config.oss_midi_device = "/dev/midi";
	config.midi_channel = 0;
	config.oss_audio_device = "/dev/dsp";
	config.alsa_audio_device = "default";
	config.sample_rate = 44100;
	config.channels = 2;
	config.buffer_size = BUF_SIZE;
	config.polyphony = 10;
	
	// load saved parameters (if any) from .amSynthrc
	string fname( getenv("HOME") );
	fname += "/.amSynthrc";
	ifstream file( fname.c_str(), ios::in );
	char buffer[100];
	while( file.good() ) {
		file >> buffer;
		if( string(buffer)=="#" ){
			// ignore lines beginning with '#' (comments)
			// this next line is needed to deal with a line with 
			// just a '#'
			file.unget();
			// this moves file on by a whole line, so we ignore it
			file.get(buffer,100);
		} else if (string(buffer)=="audio_driver"){
			file >> buffer;
			config.audio_driver = string(buffer);
		} else if (string(buffer)=="midi_driver"){
			file >> buffer;
			config.midi_driver = buffer;
		} else if (string(buffer)=="oss_midi_device"){
			file >> buffer;
			config.oss_midi_device = string(buffer);
		} else if (string(buffer)=="midi_channel"){
			file >> buffer;
			config.midi_channel = atoi(buffer);
		} else if (string(buffer)=="oss_audio_device"){
			file >> buffer;
			config.oss_audio_device = string(buffer);
		} else if (string(buffer)=="alsa_audio_device"){
			file >> buffer;
			config.alsa_audio_device = string(buffer);
		} else if (string(buffer)=="sample_rate"){
			file >> buffer;
			config.sample_rate = atoi(buffer);
		} else if (string(buffer)=="polyphony"){
			file >> buffer;
			config.polyphony = atoi(buffer);
		} else if (string(buffer)=="gui_font"){
			file >> buffer;
			load_font = 1;
			xfontname = buffer;
		} else {
			file >> buffer;
		}
	}
	file.close();
	
	presetController = new PresetController();
	
	// get command line options (they override saved prefs.)
	int opt;
	while( (opt=getopt(argc, argv, "vhm:c:a:r:p:b:"))!= -1 ) {
		switch(opt) {
			case 'm': 
				config.midi_driver = optarg;
				break;
			case 'b': 
				presetController->setBankFile( optarg );
				break;
			case 'c':
				config.midi_channel = atoi( optarg ); 
				break;
			case 'a':
				config.audio_driver = optarg; 
				break;
			case 'r':
				config.sample_rate = atoi( optarg );
				break;
			case 'p':
				config.polyphony = atoi( optarg );
				break;
			case 'v':
				cout << "amSynth " << VERSION << " -- compiled "
					<< __DATE__ << " " << __TIME__ << endl;
				return 0;
			case 'h':
				cout << help_text; 
				return 0;
			default:
				return 0;
		}
	}
#ifdef _DEBUG
	cout << "MIDI:- driver:" << config.midi_driver << " channel:" 
	<< config.midi_channel << endl << "AUDIO:- driver:" 
	<< config.audio_driver << " sample rate:" << config.sample_rate << endl;
#endif
	
	//
	// initialise audio
	//
	if (enable_audio)
	{
#ifdef with_jack
		if (config.audio_driver=="jack"||config.audio_driver=="JACK")
		{
			jack = 1;
			out = new JackOutput();
		}
		else
#endif
			out = new AudioOutput();
	
		out->setConfig( config );
	}
	
	vau = new VoiceAllocationUnit( config ); // were sure of sample_rate now
	if (enable_audio) out->setInput( *vau );
	
	presetController->loadPresets();
	
	int audio_res;
	if( enable_audio )
	{
		if (jack) out->run();
		else audio_res = 
			pthread_create(&audioThread, NULL, audio_thread, NULL);
	}
	
	
	//
	// init midi
	//
	if (enable_audio)
		midi_controller = new MidiController( config, out->getTitle() );
	else
		midi_controller = new MidiController( config, "amSynth" );

	int midi_res;
	if (enable_gui) midi_res = 
			pthread_create( &midiThread, NULL, midi_thread, NULL );
  
	// need to drop our suid-root permissions :-
	// GTK will not work SUID for security reasons..
	setreuid( getuid(), getuid() );
	setregid( getgid(), getgid() );
	
	midi_controller->setVAU( *vau );
	midi_controller->setPresetController( *presetController );
  
	vau->setPreset( presetController->getCurrentPreset() );

	presetController->selectPreset( 1 );
        presetController->selectPreset( 0 );
	
	if (enable_gui==1)
	{	
	Gtk::Main kit( &argc, &argv ); // this can be called SUID
	
	// make GDK loop read events from the pipe
	gdk_input_add( the_pipe[0], GDK_INPUT_READ, &pipe_event, (void*)NULL );

	// give audio/midi threads time to start up first..
	sleep( 1 );

	// this can be called SUID:
	gui = new GUI( config, *midi_controller, *vau, the_pipe, *out, 
			out->getTitle() );
	if (load_font) gui->set_x_font ( xfontname.c_str() );
	gui->setPresetController( *presetController );
	gui->init();

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
	xfontname = gui->get_x_font ( );
	if (load_font)
	{
		// replace fontname in .amSynthrc with new fontname
	}
	else
	{
		// create fontname entry in .amSynthrc
	}
	
	presetController->savePresets();
	midi_controller->saveConfig();
	
	if(enable_audio){
		out->stop();
		audio_res = pthread_join(audioThread, NULL);
#ifdef _DEBUG
		cout << "joined audioThread" << endl;
#endif		
	}
	
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
