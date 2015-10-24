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

#include "Configuration.h"
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
, _midiController(0)
, _presetController(0)
, _voiceAllocationUnit(0)
{
	Configuration &config = Configuration::get();

	_voiceAllocationUnit = new VoiceAllocationUnit;
	_voiceAllocationUnit->SetSampleRate(_sampleRate);
	_voiceAllocationUnit->SetMaxVoices(config.polyphony);
	_voiceAllocationUnit->setPitchBendRangeSemitones(config.pitch_bend_range);
	
	_presetController = new PresetController;
	_presetController->loadPresets(config.current_bank_file.c_str());
	_presetController->selectPreset(0);
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
	_presetController->getCurrentPreset().fromString(buffer);
}

int Synthesizer::saveState(char **buffer)
{
	std::string string = _presetController->getCurrentPreset().toString();
	return asprintf(buffer, "%s", string.c_str());
}

const char *Synthesizer::getPresetName(int presetNumber)
{
	return _presetController->getPreset(presetNumber).getName().c_str();
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
	return _presetController->getCurrentPreset().getParameter(parameter).GetNormalisedValue();
}

void Synthesizer::setParameterValue(Param parameter, float value)
{
	_presetController->getCurrentPreset().getParameter(parameter).setValue(value);
}

void Synthesizer::setNormalizedParameterValue(Param parameter, float value)
{
	_presetController->getCurrentPreset().getParameter(parameter).SetNormalisedValue(value);
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
	strncpy(buffer, _presetController->getCurrentPreset().getParameter(parameter).GetStringValue().c_str(), maxLen);
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

int Synthesizer::loadTuningKeymap(const char *filename)
{
	return _voiceAllocationUnit->loadKeyMap(filename);
}

int Synthesizer::loadTuningScale(const char *filename)
{
	return _voiceAllocationUnit->loadScale(filename);
}

void Synthesizer::defaultTuning()
{
	_voiceAllocationUnit->defaultTuning();
}

void Synthesizer::setSampleRate(int sampleRate)
{
	_sampleRate = sampleRate;
	_voiceAllocationUnit->SetSampleRate(sampleRate);
}

static bool comapare(const amsynth_midi_event_t &first, const amsynth_midi_event_t &second) { return (first.offset_frames < second.offset_frames); }

void Synthesizer::process(unsigned int nframes, std::vector<amsynth_midi_event_t> &midi_in, float *audio_l, float *audio_r, unsigned audio_stride)
{
	if (_sampleRate < 0) {
		assert(!"sample rate has not been set");
		return;
	}
	std::sort(midi_in.begin(), midi_in.end(), comapare);
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
}
