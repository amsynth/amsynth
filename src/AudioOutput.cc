/* amSynth
 * (c) 2001,2002 Nick Dowell
 **/

#include "AudioOutput.h"

AudioOutput::AudioOutput()
{
	running = 0;
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
	if ( out.open( config ) == -1) {
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
AudioOutput::run()
{
  out.setRealtime();

#ifdef _DEBUG
  cout << "<AudioOutput> entering main loop" << endl;
#endif

  running = 1;
  while (running) {
    if (out.write(input->getNFData(), BUF_SIZE*channels) == -1) exit(-1);
  }
  out.close();
}
