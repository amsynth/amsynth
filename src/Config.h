/* amSynth
 * (c) 2001,2002 Nick Dowell
 */
 
#ifndef CONFIG_H
#define CONFIG_H
 
#include <string>

class Config
{
public:
	Config();
	int sample_rate;
	int midi_channel;
	int realtime;
	int active_voices;
	int channels;
	int buffer_size;
	string midi_device;
	string audio_device;
};

#endif
