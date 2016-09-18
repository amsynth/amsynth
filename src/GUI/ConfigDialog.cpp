/*
 *  ConfigDialog.cpp
 *
 *  Copyright (c) 2001-2012 Nick Dowell
 *
 *  This file is part of amsynth.
 *
 *  amsynth is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  amsynth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with amsynth.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ConfigDialog.h"

#include "../Configuration.h"

using namespace Gtk;

ConfigDialog::ConfigDialog (Window& parent)
:	Dialog (_("amsynth configuration"), parent)
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
	
	get_vbox()->add (* manage (new Label (_("Preferred MIDI Driver"))));
	get_vbox()->add (mMidiDriver);
	get_vbox()->add (*manage (new Label (_("OSS MIDI Device"))));
	get_vbox()->add (mOSSMidiDevice);
	get_vbox()->add (*manage (new Label (_("Preferred Audio Driver"))));
	get_vbox()->add (mAudioDriver);
	get_vbox()->add (*manage (new Label (_("OSS Audio Device"))));
	get_vbox()->add (mOSSAudioDevice);
	get_vbox()->add (*manage (new Label (_("ALSA Audio Device"))));
	get_vbox()->add (mALSAAudioDevice);
	get_vbox()->add (*manage (new Label (_("Sample Rate"))));
	get_vbox()->add (mSampleRate);
	get_vbox()->add (*manage (new Label ("")));
	get_vbox()->add (*manage (new Label (_("Changes take effect after restarting amsynth"))));
	
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
	Configuration & config = Configuration::get();
	mMidiDriver.set_active_text (stringToLower (config.midi_driver));
	mOSSMidiDevice.set_text (config.oss_midi_device);
	mAudioDriver.set_active_text (stringToLower (config.audio_driver));
	mOSSAudioDevice.set_text (config.oss_audio_device);
	mALSAAudioDevice.set_text (config.alsa_audio_device);
	std::ostringstream rateStr; rateStr << config.sample_rate;
	mSampleRate.set_active_text (rateStr.str());
}

void
ConfigDialog::WriteValues ()
{
	Configuration & config = Configuration::get();
	config.midi_driver = mMidiDriver.get_active_text ();
	config.oss_midi_device = mOSSMidiDevice.get_text ();
	config.audio_driver = mAudioDriver.get_active_text ();
	config.oss_audio_device = mOSSAudioDevice.get_text ();
	config.alsa_audio_device = mALSAAudioDevice.get_text ();
	config.sample_rate = strtol (mSampleRate.get_active_text().c_str(), NULL, 0);
	config.save();
}

void
ConfigDialog::on_response (int response_id)
{
	if (RESPONSE_OK == response_id) WriteValues ();
}

