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
	
#ifdef SNDFILE_1
	// libsndfile version 1.x:
	sf_info.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;
#else
	// libsndfile versions < 1.0:
	sf_info.format = SF_FORMAT_WAV | SF_FORMAT_PCM;
	sf_info.pcmbitwidth = 16;
#endif
	
	
	if( out.open( config ) == -1)
	{
		//perror("error: could not open dsp device");
		exit(-1);
	}
}

void
AudioOutput::setInput( NFSource & source )
{
	input = &source;
}

void
AudioOutput::startRecording()
{
#ifdef SNDFILE_1
	// libsndfile version 1.x:
	snfile = sf_open( wavoutfile.c_str(), SFM_WRITE, &sf_info );
	sf_command( sndfile, SFC_SET_NORM_FLOAT, NULL, SF_TRUE );
#else
	// libsndfile versions < 1.0:
	sndfile = sf_open_write( wavoutfile.c_str(), &sf_info );
	sf_command( sndfile, "norm float", (void*)"on", 0 );
#endif
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
