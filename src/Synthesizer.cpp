/*
 *  Synthesizer.cpp
 *
 *  Copyright (c) 2014-2019 Nick Dowell
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
#include "VoiceBoard/VoiceBoard.h"

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

			if (key == PROP_KBM_FILE)
				loadTuningKeymap(value.c_str());

			if (key == PROP_SCL_FILE)
				loadTuningScale(value.c_str());
		}
	}
}

int Synthesizer::saveState(char **buffer)
{
	std::stringstream stream;
	_presetController->getCurrentPreset().toString(stream);

	const std::string &tuning_kbm_file = _voiceAllocationUnit->tuningMap.getKeyMapFile();
	if (tuning_kbm_file.length())
		stream << "<property> " PROP_KBM_FILE " " << tuning_kbm_file << std::endl;

	const std::string &tuning_scl_file = _voiceAllocationUnit->tuningMap.getScaleFile();
	if (tuning_scl_file.length())
		stream << "<property> " PROP_SCL_FILE " " << tuning_scl_file << std::endl;

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
