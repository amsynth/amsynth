/* amSynth
 * (c) 2001,2002 Nick Dowell
 */
 
#ifndef CONFIG_H
#define CONFIG_H
 
#include <string>

/**
 * @brief Encapsulates any configuration parameters which must be shared between
 * components in the system.
 */
class Config
{
public:
	Config();
	/**
	 * The sampling rate at which the output is to be produced
	 */
	int sample_rate;
	/**
	 * The number of the MIDI channel [1-16] to listen for messages on.
	 * If set to 0, all MIDI channels are listened to.
	 */
	int midi_channel;
	/**
	 * Set to 1 if the audio & midi threads are running with realtime scheduling
	 * priorities, 0 otherwise
	 */
	int realtime;
	/**
	 * A count of the number of voices currently active and producing a signal
	 */
	int active_voices;
	/**
	 * The number of audio channels to be opened by the audio driver.
	 */
	int channels;
	/**
	 * erm..
	 */
	int buffer_size;
	/**
	 * Used to specify the maximum number of voices allowed to be active 
	 * simultaneously. Attempting to play too many voices simultaneously will
	 * overload the CPU and disrupt the audio signal. Setting to 0 results in
	 * unlimited polyphony.
	 */
	int polyphony;
	/**
	 * Specify the audio output driver to use. currently "oss", "alsa", or 
	 * "auto" (which picks the best one)
	 */
	string audio_driver;
	/**
	 * Specify the midi input driver to use. currently "oss", "alsa", or 
	 * "auto" (which picks the best one)
	 */
	string midi_driver;
	/**
	 * The name if the device file for the OSS midi device.
	 */
	string oss_midi_device;
	/**
	 * The name if the device file for the OSS audio device.
	 */
	string oss_audio_device;
	/**
	 * The name of the ALSA PCM device to use
	 */
	string alsa_audio_device;
};

#endif
