/* amSynth
 * (c) 2001,2002 Nick Dowell
 **/

#include "JackOutput.h"
#include <iostream>

NFSource	*myinput;
float		*inbuf, *pt;
jack_port_t 	*l_port;
jack_port_t 	*r_port;
jack_client_t 	*client;
int		sample_rate;
int		buf_size;
int		p,q;

int
jack_process (jack_nframes_t nframes, void *arg)

{
	jack_default_audio_sample_t *lout = (jack_default_audio_sample_t *)
			jack_port_get_buffer (l_port, nframes);
	jack_default_audio_sample_t *rout = (jack_default_audio_sample_t *)
			jack_port_get_buffer (r_port, nframes);

	
	p = 0;
	while (p<buf_size)
	{
		pt = inbuf = myinput->getNFData();
		
		q = BUF_SIZE;
		while (q--)
		{
			*lout++ = *pt++;
			*rout++ = *pt++;
		}
		p += BUF_SIZE;
	}

	return 0;
}

int
jack_bufsize (jack_nframes_t nframes, void *arg)
{
	buf_size = nframes;
	return 0;
}

int
jack_srate (jack_nframes_t nframes, void *arg)
{
	sample_rate = nframes;
	return 0;
}

void
jack_shutdown (void *arg)
{
	exit (1);
}


JackOutput::JackOutput()
{
	/* try to become a client of the JACK server */
	if ( (client = jack_client_new("amSynth")) == 0 )
	{
		std::cerr << "jack server not running?\n";
		exit( 1 );
	}

	/* tell the JACK server to call `process()' whenever
	   there is work to be done.
	*/

	jack_set_process_callback (client, jack_process, 0);

	/* tell the JACK server to call `bufsize()' whenever
	   the maximum number of frames that will be passed
	   to `process()' changes
	*/

	jack_set_buffer_size_callback (client, jack_bufsize, 0);

	/* tell the JACK server to call `srate()' whenever
	   the sample rate of the system changes.
	*/


	jack_set_sample_rate_callback (client, jack_srate, 0);

	/* tell the JACK server to call `jack_shutdown()' if
	   it ever shuts down, either entirely, or if it
	   just decides to stop calling us.
	*/

	jack_on_shutdown (client, jack_shutdown, 0);

	/* display the current sample rate. once the client is activated 
	   (see below), you should rely on your own sample rate
	   callback (see above) for this value.
	*/

	sample_rate = jack_get_sample_rate( client );
	buf_size = p = jack_get_buffer_size( client );

	/* create output port */

	l_port = jack_port_register( client, "L out", 
			JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0 );
	r_port = jack_port_register( client, "R out",
			JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0 );
}

void
JackOutput::setConfig( Config & config )
{
	config.sample_rate = sample_rate;
	config.buffer_size = buf_size;
}

void
JackOutput::setInput( NFSource & source )
{
	myinput = &source;
}

void
JackOutput::startRecording()
{
}

void
JackOutput::stopRecording()
{
}

void 
JackOutput::run()
{
	if (jack_activate (client)) 
	{
		std::cerr << "cannot activate JACK client\n";
		exit (0);
	}
}

void
JackOutput::stop()
{
	jack_client_close (client);
}
