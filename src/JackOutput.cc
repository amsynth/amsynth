/* amSynth
 * (c) 2001-2007 Nick Dowell
 **/

#include "JackOutput.h"
#include "VoiceAllocationUnit.h"

#include <iostream>
#include <string.h>
#include <dlfcn.h>

#ifdef with_jack

jack_client_t*	(*dl_jack_client_new)				(const char *) = NULL;
int				(*dl_jack_client_close)				(jack_client_t *);
const char**	(*dl_jack_get_ports)				(jack_client_t *, const char *, const char *, unsigned long);
int				(*dl_jack_set_process_callback)		(jack_client_t *, JackProcessCallback, void *);
int				(*dl_jack_set_buffer_size_callback)	(jack_client_t *, JackBufferSizeCallback, void *);
int				(*dl_jack_set_sample_rate_callback)	(jack_client_t *, JackSampleRateCallback, void *);
jack_port_t*	(*dl_jack_port_register)			(jack_client_t *, const char *port_name, const char *, unsigned long, unsigned long);
void			(*dl_jack_on_shutdown)				(jack_client_t *client, void (*)(void *), void *);
jack_nframes_t	(*dl_jack_get_sample_rate)			(jack_client_t *);
jack_nframes_t	(*dl_jack_get_buffer_size)			(jack_client_t *);
int				(*dl_jack_activate)					(jack_client_t *);
int				(*dl_jack_deactivate)				(jack_client_t *);
int				(*dl_jack_connect)					(jack_client_t *, const char *, const char *);

void*			(*dl_jack_port_get_buffer)			(jack_port_t *, jack_nframes_t);
const char*		(*dl_jack_port_name)				(const jack_port_t *);

void find_library(const char * searchname, char * result, size_t size)
{
	char cmd[128]; sprintf(cmd, "ldconfig -p | grep %s", searchname);
	FILE * output = popen(cmd, "r");
	if (output)
	{
		char buffer[256] = "";
		fread(buffer, 1, 1024, output);
		pclose(output);
		// now search for a string which looks like libXXXX.so
		const char * lnBeg = strchr(buffer, 'l');
		const char * lnEnd = strchr(buffer, ' ');
		const size_t lnLen = lnEnd - lnBeg;
		strncpy(result, lnBeg, lnLen);
		result[lnLen] = '\0';
	}
	else
	{
		snprintf(result, size, "lib%s.so", searchname);
	}
}

bool load_libjack()
{
	char libjack[128] = "";
	find_library("libjack", libjack, 128);
	void* handle = dlopen(libjack, RTLD_LAZY);
	if (NULL == handle) {
		std::cerr << "cannot load JACK library (" << libjack << ")\n";
		return false;
	}
	
	dl_jack_client_new = (jack_client_t* (*) (const char *)) dlsym(handle, "jack_client_new");
	if (NULL == dl_jack_client_new) {
		std::cerr << "cannot locate 'jack_client_new'\n";
		return false;
	}
	
	dl_jack_client_close = (int (*)(jack_client_t*)) dlsym(handle, "jack_client_close");
	if (NULL == dl_jack_client_close) {
		std::cerr << "cannot locate 'jack_client_close'\n";
		return false;
	}
	
	dl_jack_get_ports = (const char** (*)(jack_client_t*, const char*, const char*, unsigned long)) dlsym(handle, "jack_get_ports");
	if (NULL == dl_jack_get_ports) {
		std::cerr << "cannot locate 'jack_get_ports'\n";
		return false;
	}
	
	dl_jack_set_process_callback = (int (*)(jack_client_t*, JackProcessCallback, void*)) dlsym(handle, "jack_set_process_callback");
	if (NULL == dl_jack_set_process_callback) {
		std::cerr << "cannot locate 'jack_set_process_callback'\n";
		return false;
	}
	
	dl_jack_set_buffer_size_callback = (int (*)(jack_client_t*, JackBufferSizeCallback bufsize, void*)) dlsym(handle, "jack_set_buffer_size_callback");
	if (NULL == dl_jack_set_buffer_size_callback) {
		std::cerr << "cannot locate 'jack_set_buffer_size_callback'\n";
		return false;
	}
	
	dl_jack_set_sample_rate_callback = (int (*)(jack_client_t*, JackSampleRateCallback, void*)) dlsym(handle, "jack_set_sample_rate_callback");
	if (NULL == dl_jack_set_sample_rate_callback) {
		std::cerr << "cannot locate 'jack_set_sample_rate_callback'\n";
		return false;
	}
	
	dl_jack_port_register = (jack_port_t* (*)(jack_client_t*, const char*, const char*, unsigned long, unsigned long)) dlsym(handle, "jack_port_register");
	if (NULL == dl_jack_port_register) {
		std::cerr << "cannot locate 'jack_port_register'\n";
		return false;
	}
	
	dl_jack_on_shutdown = (void	(*) (jack_client_t *client, void (*)(void *), void *)) dlsym(handle, "jack_on_shutdown");
	if (NULL == dl_jack_on_shutdown) {
		std::cerr << "cannot locate 'jack_on_shutdown'\n";
		return false;
	}
	
	dl_jack_get_sample_rate = (jack_nframes_t (*)(jack_client_t *)) dlsym(handle, "jack_get_sample_rate");
	if (NULL == dl_jack_get_sample_rate) {
		std::cerr << "cannot locate 'jack_get_sample_rate'\n";
		return false;
	}
	
	dl_jack_get_buffer_size = (jack_nframes_t (*)(jack_client_t *)) dlsym(handle, "jack_get_buffer_size");
	if (NULL == dl_jack_get_buffer_size) {
		std::cerr << "cannot locate 'jack_get_buffer_size'\n";
		return false;
	}

	dl_jack_activate = (int (*)(jack_client_t *)) dlsym(handle, "jack_activate");
	if (NULL == dl_jack_activate) {
		std::cerr << "cannot locate 'jack_activate'\n";
		return false;
	}

	dl_jack_deactivate = (int (*)(jack_client_t *)) dlsym(handle, "jack_deactivate");
	if (NULL == dl_jack_deactivate) {
		std::cerr << "cannot locate 'jack_deactivate'\n";
		return false;
	}
	
	dl_jack_connect = (int (*)(jack_client_t *, const char *, const char *)) dlsym(handle, "jack_connect");
	if (NULL == dl_jack_connect) {
		std::cerr << "cannot locate 'jack_connect'\n";
		return false;
	}
	
	dl_jack_port_get_buffer = (void* (*)(jack_port_t *, jack_nframes_t)) dlsym(handle, "jack_port_get_buffer");
	if (NULL == dl_jack_port_get_buffer) {
		std::cerr << "cannot locate 'jack_port_get_buffer'\n";
		return false;
	}
	
	dl_jack_port_name = (const char* (*)(const jack_port_t *)) dlsym(handle, "jack_port_name");
	if (NULL == dl_jack_port_name) {
		std::cerr << "cannot locate 'jack_port_name'\n";
		return false;
	}

	
	std::cerr << "loaded & initialised " << libjack << " :)\n";
	return true;
}

int
jack_process (jack_nframes_t nframes, void *arg)
{
	JackOutput *client = (JackOutput *) arg;
	return client->process (nframes);
}

int
jack_bufsize (jack_nframes_t nframes, void *arg)
{
	JackOutput *client = (JackOutput *) arg;
	return client->bufsize (nframes);
}

int
jack_srate (jack_nframes_t nframes, void *arg)
{
	JackOutput *client = (JackOutput *) arg;
	return client->srate (nframes);
}

void
jack_shutdown (void *arg)
{
	JackOutput *client = (JackOutput *) arg;
	client->shutdown ();
}

void
jack_error	(const char* msg)
{
	std::cerr << msg << std::endl;
}

#endif

int
JackOutput::init	( Config & config )
{
#ifdef with_jack
	initialised = false;
	client_name = "amSynth";
	
	if (!load_libjack()) return -1;
	
	
	// check if there are already any amSynth jack clients...
	
	const char **readports;
	
	if ( (client = (*dl_jack_client_new)( "amSynth-tmp" )) == 0 ) return -1;

	readports = (*dl_jack_get_ports)( client, NULL, NULL, JackPortIsOutput );
	
	int i=0, c=0;
	while (readports && readports[i])
	{
		if (strncmp( readports[i], "amSynth", 7 )==0) c++;
		i++;
	}
	c/=2;
	(*dl_jack_client_close)(client);
	if (c>0)
	{
		char tmp[3];
		sprintf( tmp, "%d", c );
		
		client_name += " (";
		client_name += string( tmp );
		client_name += ")";
	}
	
	/* become a client of the JACK server */
	if ((client = (*dl_jack_client_new)(client_name.c_str())) == 0)
	{
		error_msg = "jack_client_new() failed";
		return -1;
	}

	(*dl_jack_set_process_callback)(client, jack_process, this);
	(*dl_jack_set_buffer_size_callback)(client, jack_bufsize, this);
	(*dl_jack_set_sample_rate_callback)(client, jack_srate, this);
	(*dl_jack_on_shutdown)(client, jack_shutdown, this);

	sample_rate = (*dl_jack_get_sample_rate)( client );
	buf_size = (*dl_jack_get_buffer_size)(client);

	/* create output ports */
	l_port = (*dl_jack_port_register)(client, "L out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	r_port = (*dl_jack_port_register)(client, "R out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	
	initialised = true;
	
	config.sample_rate = sample_rate;
	config.buffer_size = buf_size;
	config.current_audio_driver = "JACK";
	
	return 0;
#endif
	return -1;
}

#ifdef with_jack
int
JackOutput::process (jack_nframes_t nframes)
{
	float *lout = (jack_default_audio_sample_t *) (*dl_jack_port_get_buffer)(l_port, nframes);
	float *rout = (jack_default_audio_sample_t *) (*dl_jack_port_get_buffer)(r_port, nframes);
	mInput->Process (lout, rout, nframes);
	return 0;
}

int
JackOutput::bufsize (jack_nframes_t nframes)
{
	buf_size = nframes;
	return 0;
}

int
JackOutput::srate (jack_nframes_t nframes)
{
	sample_rate = nframes;
	return 0;
}

void
JackOutput::shutdown ()
{
}
#endif

bool 
JackOutput::Start	()
{
#ifdef with_jack
	if (!initialised) return false;
	if (!mInput) return false;
	if ((*dl_jack_activate)(client)) 
	{
		std::cerr << "cannot activate JACK client\n";
		return false;
	}
	(*dl_jack_connect)(client, (*dl_jack_port_name)(l_port), "alsa_pcm:playback_1");
	(*dl_jack_connect)(client, (*dl_jack_port_name)(r_port), "alsa_pcm:playback_2");
	return true;
#else
	return false;
#endif
}

void
JackOutput::Stop()
{
#ifdef with_jack
	if (!initialised) return;
	(*dl_jack_deactivate)(client);
	(*dl_jack_client_close)(client);
#endif
}
