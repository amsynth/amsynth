/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "main.h"

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
	
	int enable_audio = 1;
	// set default parameters
	config.midi_device = "/dev/midi";
	config.midi_channel = 0;
	config.audio_device = "/dev/dsp";
	config.sample_rate = 44100;
	config.channels = 2;
	config.buffer_size = BUF_SIZE;
	config.polyphony = 10;
	
	// load saved parameters (if any) from .amSynthrc
	string fname(getenv("HOME"));
	fname += "/.amSynthrc";
	ifstream file(fname.c_str(), ios::in);
	char buffer[100];
	while( file.good() ) {
		file >>  buffer;
		if (string(buffer)=="#"){
			// ignore lines beginning with '#' (comments)
			file.get(buffer,100);
		} else if (string(buffer)=="midi_device"){
			file >> buffer;
			config.midi_device = string(buffer);
		} else if (string(buffer)=="midi_channel"){
			file >> buffer;
			config.midi_channel = atoi(buffer);
		} else if (string(buffer)=="audio_device"){
			file >> buffer;
			config.audio_device = string(buffer);
		} else if (string(buffer)=="sample_rate"){
			file >> buffer;
			config.sample_rate = atoi(buffer);
		} else if (string(buffer)=="polyphony"){
			file >> buffer;
			config.polyphony = atoi(buffer);
		} else {
			file >> buffer;
		}
	}
	file.close();
	
	// get command line options (they override saved prefs.)
	int opt;
	while((opt=getopt(argc, argv, "svhm:c:d:r:p:"))!= -1) {
		switch(opt) {
			case 's':
				enable_audio = 0;
				break;
			case 'm': 
				config.midi_device = optarg; 
				break;
			case 'c':
				config.midi_channel = atoi(optarg); 
				break;
			case 'd':
				config.audio_device = optarg; 
				break;
			case 'r':
				config.sample_rate = atoi(optarg);
				break;
			case 'p':
				config.polyphony = atoi(optarg); 
				break;
			case 'v':
				cout << "amSynth version " << VERSION << endl 
				<< "compiled " << __DATE__ << " " << __TIME__ << endl;
				return 0;
			case 'h':
				cout << help_text; 
				return 0;
			default:
				return 0;
		}
	}
#ifdef _DEBUG
	cout << "MIDI:- device:" << config.midi_device << " channel:" 
	<< config.midi_channel << endl << "AUDIO:- device:" 
	<< config.audio_device << " sample rate:" << config.sample_rate << endl;
#endif
	presetController = new PresetController();
	midi_controller = new MidiController();
	midi_controller->setConfig( config );
	out = new AudioOutput();
	out->setConfig( config );
	vau = new VoiceAllocationUnit( config ); //after were sure of sample_rate
	out->setInput(*vau);
	
	presetController->loadPresets();
	
	int audio_res;
	if(enable_audio)
		audio_res = pthread_create(&audioThread, NULL, audio_thread, NULL);
	int midi_res;
	midi_res = pthread_create(&midiThread, NULL, midi_thread, NULL);
  
	// need to drop our suid-root permissions :-
	// GTK will not work SUID for security reasons..
	setuid(getuid());
	seteuid(getuid());
	
	midi_controller->setVAU(*vau);
	midi_controller->setPresetController(*presetController);
  
	vau->setPreset(presetController->getCurrentPreset());
	
	Gtk::Main kit(&argc, &argv); // this can be called SUID
	
	// make GDK loop read events from the pipe
	gdk_input_add( the_pipe[0], GDK_INPUT_READ, &pipe_event, (void*)NULL );
	
	gui = new GUI( config, *midi_controller, the_pipe ); // this can be called SUID
	gui->setPresetController(*presetController);
	gui->init();
	kit.run(); // this _cannot_ be run SUID

#ifdef _DEBUG
	cout << "main() : GUI was terminated, shutting down cleanly.." << endl;
#endif
	
	/*
	 * code to shut down cleanly..
	 */
	
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
