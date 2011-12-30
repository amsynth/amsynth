/* amSynth
 * (c) 2001-2006 Nick Dowell
 **/

#if HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "OSSMidiDriver.h"

class OSSMidiDriver : public MidiDriver
{
public:
	OSSMidiDriver();
  	virtual ~OSSMidiDriver();
	
	int open( Config & config );
	int close();
	
	int read(unsigned char *bytes, unsigned maxBytes);
	int write_cc(unsigned int channel, unsigned int param, unsigned int value);
	
private:
		int _fd;
};


OSSMidiDriver::OSSMidiDriver()
:	_fd(-1)
{
}

OSSMidiDriver::~OSSMidiDriver()
{
    close();
}

int OSSMidiDriver::open( Config & config )
{
	if (_fd == -1) 
	{
		_fd = ::open(config.oss_audio_device.c_str(), O_RDONLY, 0);
	}
	return (_fd > -1) ? 0 : -1;
}

int OSSMidiDriver::close()
{
	if (_fd > -1)
	{
		::close(_fd);
		_fd = -1;
	}
	return 0;
}

int OSSMidiDriver::read(unsigned char *bytes, unsigned maxBytes)
{
    return ::read(_fd, bytes, maxBytes);
}


int OSSMidiDriver::write_cc(unsigned int /*channel*/, unsigned int /*param*/, unsigned int /*value*/)
{
    return -1;
}

MidiDriver* CreateOSSMidiDriver() { return new OSSMidiDriver; }
