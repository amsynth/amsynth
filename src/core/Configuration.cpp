/*
 *  Configuration.cpp
 *
 *  Copyright (c) 2001-2020 Nick Dowell
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
 
#include "Configuration.h"

#include "filesystem.h"

#include <fstream>
#include <iostream>
#include <cstdlib>
#include <sstream>

static std::string to_string(double value)
{
	std::ostringstream ostr;
	ostr.imbue(std::locale::classic());
	ostr << value;
	return ostr.str();
}

Configuration::Configuration()
{
	amsynthrc_fname = filesystem::get().config;
	sample_rate = midi_channel = polyphony = xruns = 0;
#ifdef ENABLE_REALTIME
	realtime = 0;
#endif
	Defaults();
	load();
}

void
Configuration::Defaults	()
{
	audio_driver = "auto";
	midi_driver = "auto";
	oss_midi_device = "/dev/midi";
	midi_channel = 0;
	oss_audio_device = "/dev/dsp";
	alsa_audio_device = "default";
	sample_rate = 44100;
	channels = 2;
	buffer_size = 128;
	polyphony = 10;
	pitch_bend_range = 2;
	locked_parameters.clear();
	jack_autoconnect = true;
	ui_scale = 0.0;
	jack_client_name_preference = "amsynth";
	current_bank_file = filesystem::get().default_bank;
	current_tuning_file = "default";
}

int
Configuration::load	()
{
	std::ifstream file(amsynthrc_fname.c_str());
	if (!file) {
		return -1;
	}

	std::string line;
	while (std::getline(file, line)) {
		std::istringstream istr(line);
		istr.imbue(std::locale::classic());
		std::string buffer;
		istr >> buffer;
		if (!buffer.empty() && buffer[0] == '#') {
			continue;
		} else if (buffer=="audio_driver"){
			istr >> audio_driver;
		} else if (buffer=="midi_driver"){
			istr >> midi_driver;
		} else if (buffer=="oss_midi_device"){
			istr >> oss_midi_device;
		} else if (buffer=="midi_channel"){
			istr >> midi_channel;
		} else if (buffer=="oss_audio_device"){
			istr >> oss_audio_device;
		} else if (buffer=="alsa_audio_device"){
			istr >> alsa_audio_device;
		} else if (buffer=="sample_rate"){
			istr >> sample_rate;
		} else if (buffer=="polyphony"){
			istr >> polyphony;
		} else if (buffer=="pitch_bend_range"){
			istr >> pitch_bend_range;
		} else if (buffer=="tuning_file") {
			istr >> current_tuning_file;
		} else if (buffer == "ignored_parameters" || buffer == "locked_parameters") {
			locked_parameters.clear();
			while (istr >> buffer) {
				locked_parameters += buffer;
				locked_parameters += " ";
			}
		} else if (buffer == "jack_autoconnect" && istr >> buffer) {
			jack_autoconnect = (buffer == "true");
		} else if (buffer == "ui_scale") {
			istr >> ui_scale;
		}
	}
	file.close();
	
	return 0;
}

int
Configuration::save	()
{
	FILE *fout = fopen (amsynthrc_fname.c_str(), "w");
	if (nullptr == fout)
		return -1;

	fprintf (fout, "# Audio device type [auto | jack | alsa-mmap | alsa | oss]\n");
	fprintf (fout, "# \"jack\" is best for low latency and interoperability\n");
	fprintf (fout, "audio_driver %s\n\n", audio_driver.c_str());

	fprintf (fout, "# ALSA: PCM device name, e.g. hw:0\n");
	fprintf (fout, "alsa_audio_device %s\n\n", alsa_audio_device.c_str());

	fprintf (fout, "# OSS: audio device path, e.g. /dev/dsp\n");
	fprintf (fout, "oss_audio_device %s\n\n", oss_audio_device.c_str());

	fprintf (fout, "# JACK: automatically connect to first physical audio outputs & all MIDI inputs [true | false]\n");
	fprintf (fout, "jack_autoconnect %s\n\n", jack_autoconnect ? "true" : "false");

	fprintf (fout, "# Audio sampling rate when using ALSA or OSS)\n");
	fprintf (fout, "sample_rate %d\n\n", sample_rate);

	fprintf (fout, "# MIDI device type [auto | alsa | oss]\n");
	fprintf (fout, "midi_driver %s\n\n", midi_driver.c_str());

	fprintf (fout, "# MIDI channel for input & output [1 - 16 | 0 (all channels)]\n");
	fprintf (fout, "midi_channel %d\n\n", midi_channel);

	fprintf (fout, "# OSS: MIDI device path, e.g. /dev/midi\n");
	fprintf (fout, "oss_midi_device %s\n\n", oss_midi_device.c_str());

	fprintf (fout, "# Maximum polyphony (0 = unlimited)\n");
	fprintf (fout, "polyphony %d\n\n", polyphony);

	fprintf (fout, "# Pitch bend range in semitones\n");
	fprintf (fout, "pitch_bend_range %d\n\n", pitch_bend_range);

	fprintf (fout, "# Tuning file\n");
	fprintf (fout, "tuning_file %s\n\n", current_tuning_file.c_str());

	fprintf (fout, "# Parameter names that should be locked during preset load\n");
	fprintf (fout, "locked_parameters %s\n\n", locked_parameters.c_str());

	fprintf (fout, "# Additional UI scaling factor\n");
	fprintf (fout, "ui_scale %s\n\n", to_string(ui_scale).c_str());

	fclose (fout);
	return 0;
}
