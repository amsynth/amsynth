/* Synth--
 * (c) 2001 Nick Dowell
 **/

#include "ALSAMidiDriver.h"

int
 ALSAMidiDriver::read(unsigned char *midi_event_buffer)
{
#ifdef _ASLA
  _bytes_read = snd_rawmidi_read(_alsa_midi_handle, 
				 midi_event_buffer, MIDI_BUF_SIZE);
  return _bytes_read;
#else
  return -1;
#endif
}

int ALSAMidiDriver::close()
{
  return 0;
}

int ALSAMidiDriver::open(string device)
{
#ifdef _ALSA
  int ret = snd_rawmidi_open(&_alsa_midi_handle, _alsa_midi_card,
			     _alsa_midi_device, SND_RAWMIDI_OPEN_INPUT);
  return ret;
#else
  return -1;
#endif
}

ALSAMidiDriver::ALSAMidiDriver()
{
  _alsa_midi_card = 0;
  _alsa_midi_device = 0;
}

ALSAMidiDriver::~ALSAMidiDriver()
{
  close();
}

/* set to non-blocking mode */
//    if ((snd_rawmidi_block_mode(midi_handle, 0)) == 0)
//      printf("set to non-block mode\n");
//    else {
//      printf("error setting midi to non-blocking\n");
//      exit(-1);
//    }
