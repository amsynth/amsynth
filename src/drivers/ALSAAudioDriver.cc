/* Synth--
 * (c) 2001 Nick Dowell
 **/

#include "ALSAAudioDriver.h"

int
ALSAAudioDriver::write(float *buffer, int frames)
{
  // WRITE ME!
  frames = 0;
  buffer = 0;
  return -1;
}

int ALSAAudioDriver::open()
{
  // WRITE ME!
  return -1;
}

void ALSAAudioDriver::close()
{
  // WRITE ME!
}

int ALSAAudioDriver::setChannels(int channels)
{
  // WRITE ME!
  _channels = channels;
  return 0;
}

int ALSAAudioDriver::setRate(int rate)
{
  // WRITE ME!
  _rate = rate;
  return 0;
}

int ALSAAudioDriver::setRealtime()
{
  // WRITE ME!
  return 0;
}

ALSAAudioDriver::ALSAAudioDriver()
{
  // WRITE ME!
}

ALSAAudioDriver::~ALSAAudioDriver()
{
  close();
}
