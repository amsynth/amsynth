/* amSynth
 * (c) 2001,2002 Nick Dowell
 **/

#include "AudioOutput.h"

AudioOutput::AudioOutput()
{
	running = 0;
	recording = 0;
	wavoutfile = "/tmp/amSynth.wav";
}

AudioOutput::~AudioOutput()
{
    out.close();
}

void
AudioOutput::setConfig( Config & config )
{
	this->config = &config;
	channels = config.channels;
	
	sf_info.samplerate = config.sample_rate;
	sf_info.channels = config.channels;
	sf_info.format = SF_FORMAT_WAV | SF_FORMAT_PCM;
	sf_info.pcmbitwidth = 16;
	
	if( out.open( config ) == -1)
	{
		//perror("error: could not open dsp device");
		exit(-1);
	}
}

void
AudioOutput::setInput(NFSource & source)
{
    input = &source;
}

void
AudioOutput::startRecording()
{
	sndfile = sf_open_write( wavoutfile.c_str(), &sf_info );
	sf_command( sndfile, "norm float", (void*)"on", 0 );
	recording = 1;
}

void
AudioOutput::stopRecording()
{
	recording = 0;
	sf_close( sndfile );
}

void 
AudioOutput::run()
{
	out.setRealtime();

#ifdef _DEBUG
	cout << "<AudioOutput> entering main loop" << endl;
#endif
	
	running = 1;
	while (running)
	{
		float *buffer = input->getNFData();
		if( recording )
			sf_writef_float( sndfile, buffer, BUF_SIZE );
		if( out.write( buffer, BUF_SIZE*channels ) == -1 )
			exit(-1);
	}
	
	out.close();
}
