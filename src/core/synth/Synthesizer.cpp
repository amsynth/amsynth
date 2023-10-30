/*
 *  Synthesizer.cpp
 *
 *  Copyright (c) 2014 Nick Dowell
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

#include "Synthesizer.h"

#include "MidiController.h"
#include "PresetController.h"
#include "VoiceAllocationUnit.h"
#include "VoiceBoard.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>


Synthesizer::Synthesizer()
: _sampleRate(-1)
, _midiController(nullptr)
, _presetController(nullptr)
, _voiceAllocationUnit(nullptr)
{
	_voiceAllocationUnit = new VoiceAllocationUnit;
	_voiceAllocationUnit->SetSampleRate((int) _sampleRate);

	_presetController = new PresetController;
	_presetController->getCurrentPreset().AddListenerToAll(_voiceAllocationUnit);
	for (const auto &bank : PresetController::getPresetBanks()) {
		if (bank.file_path == _presetController->getFilePath()) {
			propertyStore_[PROP_NAME(preset_bank_name)] = bank.name;
			break;
		}
	}
	propertyStore_[PROP_NAME(preset_name)] = _presetController->getCurrentPreset().getName();
	propertyStore_[PROP_NAME(preset_number)] = std::to_string(_presetController->getCurrPresetNumber());

	_midiController = new MidiController();
	_midiController->SetMidiEventHandler(_voiceAllocationUnit);
	_midiController->setPresetController(*_presetController);
}

Synthesizer::~Synthesizer()
{
	delete _midiController;
	delete _presetController;
	delete _voiceAllocationUnit;
}

void Synthesizer::setProperty(const char *name, const char *value)
{
	if (value && strlen(value))
		propertyStore_[name] = value;
	else
		propertyStore_.erase(name);

	if (name == std::string(PROP_NAME(max_polyphony)))
		setMaxNumVoices(std::stoi(value));

	if (name == std::string(PROP_NAME(midi_channel)))
		setMidiChannel(std::stoi(value));

	if (name == std::string(PROP_NAME(pitch_bend_range)))
		setPitchBendRangeSemitones(std::stoi(value));

	if (name == std::string(PROP_NAME(tuning_kbm_file)))
		loadTuningKeymap(value);

	if (name == std::string(PROP_NAME(tuning_scl_file)))
		loadTuningScale(value);
	
	if (name == std::string(PROP_NAME(tuning_mts_esp_disabled)))
		_voiceAllocationUnit->mtsEspDisabled = std::stoi(value);
}

std::map<std::string, std::string> Synthesizer::getProperties()
{
	auto props = propertyStore_;
	props[PROP_NAME(max_polyphony)] = std::to_string(getMaxNumVoices());
	props[PROP_NAME(midi_channel)] = std::to_string(getMidiChannel());
	props[PROP_NAME(pitch_bend_range)] = std::to_string(getPitchBendRangeSemitones());
	if (!_voiceAllocationUnit->tuningMap.getKeyMapFile().empty())
		props[PROP_NAME(tuning_kbm_file)] = _voiceAllocationUnit->tuningMap.getKeyMapFile();
	if (!_voiceAllocationUnit->tuningMap.getScaleFile().empty())
		props[PROP_NAME(tuning_scl_file)] = _voiceAllocationUnit->tuningMap.getScaleFile();
#ifdef WITH_MTS_ESP
	props[PROP_NAME(tuning_mts_esp_disabled)] = _voiceAllocationUnit->mtsEspDisabled ? "1" : "0";
#endif
	return props;
}

void Synthesizer::loadBank(const char *filename)
{
	_presetController->loadPresets(filename);
	_presetController->selectPreset(_presetController->getCurrPresetNumber());
}

void Synthesizer::saveBank(const char *filename)
{
	_presetController->commitPreset();
	_presetController->savePresets(filename);
}

void Synthesizer::loadState(char *buffer)
{
	if (!_presetController->getCurrentPreset().fromString(buffer))
		return;

	std::istringstream input (buffer);
	for (std::string line; std::getline(input, line); ) {
		std::istringstream stream (line);
		std::string type, key, value;
		stream >> type;

		if (type == "<property>") {
			stream >> key;
			stream.get(); // skip whitespace
			std::getline(stream, value); // value may contain whitespace
			setProperty(key.c_str(), value.c_str());
		}
	}
}

int Synthesizer::saveState(char **buffer)
{
	std::stringstream stream;
	_presetController->getCurrentPreset().toString(stream);

	for (auto &it : getProperties())
		stream << "<property> " << it.first << " " << it.second << std::endl;

	std::string string = stream.str();
	*buffer = (char *)malloc(4096);
	return sprintf(*buffer, "%s", string.c_str());
}

int Synthesizer::getPresetNumber()
{
	return _presetController->getCurrPresetNumber();
}

void Synthesizer::setPresetNumber(int number)
{
	_presetController->selectPreset(number);
}

float Synthesizer::getParameterValue(Param parameter)
{
	return _presetController->getCurrentPreset().getParameter(parameter).getValue();
}

float Synthesizer::getNormalizedParameterValue(Param parameter)
{
	return _presetController->getCurrentPreset().getParameter(parameter).getNormalisedValue();
}

void Synthesizer::setParameterValue(Param parameter, float value)
{
	_presetController->getCurrentPreset().getParameter(parameter).setValue(value);
}

void Synthesizer::setNormalizedParameterValue(Param parameter, float value)
{
	_presetController->getCurrentPreset().getParameter(parameter).setNormalisedValue(value);
}

void Synthesizer::getParameterName(Param parameter, char *buffer, size_t maxLen)
{
	strncpy(buffer, _presetController->getCurrentPreset().getParameter(parameter).getName().c_str(), maxLen);
}

void Synthesizer::getParameterLabel(Param parameter, char *buffer, size_t maxLen)
{
	strncpy(buffer, _presetController->getCurrentPreset().getParameter(parameter).getLabel().c_str(), maxLen);
}

void Synthesizer::getParameterDisplay(Param parameter, char *buffer, size_t maxLen)
{
	strncpy(buffer, _presetController->getCurrentPreset().getParameter(parameter).getStringValue().c_str(), maxLen);
}

int Synthesizer::getPitchBendRangeSemitones()
{
	return _voiceAllocationUnit->getPitchBendRangeSemitones();
}

void Synthesizer::setPitchBendRangeSemitones(int value)
{
	_voiceAllocationUnit->setPitchBendRangeSemitones(value);
}

int Synthesizer::getMaxNumVoices()
{
	return _voiceAllocationUnit->GetMaxVoices();
}

void Synthesizer::setMaxNumVoices(int value)
{
	_voiceAllocationUnit->SetMaxVoices(value);
}

unsigned char Synthesizer::getMidiChannel()
{
	return _midiController->assignedChannel;
}

void Synthesizer::setMidiChannel(unsigned char channel)
{
	_midiController->assignedChannel = channel;
	if (channel != kMidiChannel_Any) {
		// A reset is required when switching to a new channel since we will
		// not receive the note off events for currently held notes.
		needsResetAllVoices_ = true;
	}
}

int Synthesizer::loadTuningKeymap(const char *filename)
{
	if (filename && strlen(filename))
		return _voiceAllocationUnit->loadKeyMap(filename);

	_voiceAllocationUnit->tuningMap.defaultKeyMap();
	return 0;
}

int Synthesizer::loadTuningScale(const char *filename)
{
	if (filename && strlen(filename))
		return _voiceAllocationUnit->loadScale(filename);

	_voiceAllocationUnit->tuningMap.defaultScale();
	return 0;
}

void Synthesizer::setSampleRate(int sampleRate)
{
	_sampleRate = sampleRate;
	_voiceAllocationUnit->SetSampleRate(sampleRate);
}

void Synthesizer::process(unsigned int nframes,
						  const std::vector<amsynth_midi_event_t> &midi_in,
						  std::vector<amsynth_midi_cc_t> &midi_out,
						  float *audio_l, float *audio_r, unsigned audio_stride)
{
	if (_sampleRate < 0) {
		assert(nullptr == "sample rate has not been set");
		return;
	}
	if (needsResetAllVoices_) {
		needsResetAllVoices_ = false;
		_voiceAllocationUnit->resetAllVoices();
	}
	std::vector<amsynth_midi_event_t>::const_iterator event = midi_in.begin();
	unsigned frames_left_in_buffer = nframes, frame_index = 0;
	while (frames_left_in_buffer) {
		while (event != midi_in.end() && event->offset_frames <= frame_index) {
			_midiController->HandleMidiData(event->buffer, event->length);
			++event;
		}
		
		unsigned block_size_frames = std::min(frames_left_in_buffer, (unsigned)VoiceBoard::kMaxProcessBufferSize);
		if (event != midi_in.end() && event->offset_frames > frame_index) {
			unsigned frames_until_next_event = event->offset_frames - frame_index;
			block_size_frames = std::min(block_size_frames, frames_until_next_event);
		}
		
		_voiceAllocationUnit->Process(audio_l + (frame_index * audio_stride),
									  audio_r + (frame_index * audio_stride),
									  block_size_frames, audio_stride);
		
		frame_index += block_size_frames;
		frames_left_in_buffer -= block_size_frames;
	}
	while (event != midi_in.end()) {
		_midiController->HandleMidiData(event->buffer, event->length);
		++event;
	}
	_midiController->generateMidiOutput(midi_out);
}
