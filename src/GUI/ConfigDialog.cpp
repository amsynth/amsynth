/*
 * ConfigDialog.h
 * amSynth
 * (C) 2006 Nick Dowell
 */

#include "ConfigDialog.h"

using namespace Gtk;

ConfigDialog::ConfigDialog (Window& parent, Config & config)
:	Dialog ("amSynth configuration", parent)
,	mConfig (config)
{
	mMidiDriver.append_text ("auto");
	mMidiDriver.append_text ("alsa");
	mMidiDriver.append_text ("oss");
	
	mAudioDriver.append_text ("auto");
	mAudioDriver.append_text ("jack");
	mAudioDriver.append_text ("alsa-mmap");
	mAudioDriver.append_text ("alsa");
	mAudioDriver.append_text ("oss");
	
	mSampleRate.append_text ("22050");
	mSampleRate.append_text ("44100");
	mSampleRate.append_text ("48000");
	mSampleRate.append_text ("88200");
	mSampleRate.append_text ("96000");
	mSampleRate.append_text ("176400");
	mSampleRate.append_text ("192000");
	
	get_vbox()->add (* manage (new Label ("Preferred MIDI Driver")));
	get_vbox()->add (mMidiDriver);
	get_vbox()->add (*manage (new Label ("OSS MIDI Device")));
	get_vbox()->add (mOSSMidiDevice);
	get_vbox()->add (*manage (new Label ("Preferred Audio Driver")));
	get_vbox()->add (mAudioDriver);
	get_vbox()->add (*manage (new Label ("OSS Audio Device")));
	get_vbox()->add (mOSSAudioDevice);
	get_vbox()->add (*manage (new Label ("ALSA Audio Device")));
	get_vbox()->add (mALSAAudioDevice);
	get_vbox()->add (*manage (new Label ("Sample Rate")));
	get_vbox()->add (mSampleRate);
	get_vbox()->add (*manage (new Label ("")));
	get_vbox()->add (*manage (new Label ("Changes take effect after restarting amSynth")));
	
	ReadValues();
	
	add_button (Stock::OK, RESPONSE_OK);
	add_button (Stock::CANCEL, RESPONSE_CANCEL);
	
	show_all_children();
}

#include <cctype>
std::string stringToLower(std::string myString)
{
	std::transform (myString.begin(), myString.end(), myString.begin(), tolower);
	return myString;
}

void
ConfigDialog::ReadValues ()
{
	mMidiDriver.set_active_text (stringToLower (mConfig.midi_driver));
	mOSSMidiDevice.set_text (mConfig.oss_midi_device);
	mAudioDriver.set_active_text (stringToLower (mConfig.audio_driver));
	mOSSAudioDevice.set_text (mConfig.oss_audio_device);
	mALSAAudioDevice.set_text (mConfig.alsa_audio_device);
	static char rateStr[10]; sprintf (rateStr, "%d", mConfig.sample_rate);
	mSampleRate.set_active_text (rateStr);
}

void
ConfigDialog::WriteValues ()
{
	mConfig.midi_driver = mMidiDriver.get_active_text ();
	mConfig.oss_midi_device = mOSSMidiDevice.get_text ();
	mConfig.audio_driver = mAudioDriver.get_active_text ();
	mConfig.oss_audio_device = mOSSAudioDevice.get_text ();
	mConfig.alsa_audio_device = mALSAAudioDevice.get_text ();
	mConfig.sample_rate = strtol (mSampleRate.get_active_text().c_str(), NULL, 0);
	mConfig.save();
}

void
ConfigDialog::on_response (int response_id)
{
	if (RESPONSE_OK == response_id) WriteValues ();
}

