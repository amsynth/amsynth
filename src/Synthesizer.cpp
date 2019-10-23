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

#include "Configuration.h"
#include "MidiController.h"
#include "PresetController.h"
#include "VoiceAllocationUnit.h"
#include "VoiceBoard/VoiceBoard.h"

#include <algorithm>
#include <cassert>
#include <cstdio>
#include <cstring>

#ifdef WITH_RTOSC
#include <rtosc/rtosc.h>
#include <rtosc/ports.h>
#include <rtosc/port-sugar.h>

// The boilerplate macro will cast the void pointer in the callback to "rObject".
#define rObject Synthesizer

static rtosc::Ports ports = {
    {"/amsynth/loadBank:s", rDoc("Load a bank file"), 0,
        [](const char *m,rtosc::RtData &d){
            ((Synthesizer *)d.obj)->loadBank(rtosc_argument(m,0).s);}},
//    {"/amsynth/loadState:s", rDoc("Load the state from a string"), 0,
//        [](const char *m,rtosc::RtData &d){
//            ((Synthesizer *)d.obj)->loadState(rtosc_argument(m,0).s);}},
    {"/amsynth/selectPreset:i", rDoc("Load a preset by its index"), 0,
        [](const char *m,rtosc::RtData &d){
            ((Synthesizer *)d.obj)->setPresetNumber(rtosc_argument(m,0).i);}},
    {"/amsynth/loadTuningKeymap:s", rDoc("Load a keymap file"), 0,
        [](const char *m,rtosc::RtData &d){
            ((Synthesizer *)d.obj)->loadTuningKeymap(rtosc_argument(m,0).s);}},
    {"/amsynth/loadTuningScale:s", rDoc("Load a scale file"), 0,
        [](const char *m,rtosc::RtData &d){
            ((Synthesizer *)d.obj)->loadTuningScale(rtosc_argument(m,0).s);}},

    {"/amsynth/noteOn:if", rDoc("Turn a note on with velocity"), 0,
        [](const char *m,rtosc::RtData &d){
            ((Synthesizer *)d.obj)->_voiceAllocationUnit->HandleMidiNoteOn(rtosc_argument(m,0).i, rtosc_argument(m,1).f);}},
    {"/amsynth/keyOn:iff", rDoc("Turn a key on with velocity and frequency"), 0,
        [](const char *m,rtosc::RtData &d){
            ((Synthesizer *)d.obj)->_voiceAllocationUnit->HandleNoteOn(rtosc_argument(m,0).i, rtosc_argument(m,1).f, rtosc_argument(m,2).f);}},
    {"/amsynth/noteOff:if", rDoc("Turn a note off with velocity"), 0,
        [](const char *m,rtosc::RtData &d){
            ((Synthesizer *)d.obj)->_voiceAllocationUnit->HandleMidiNoteOff(rtosc_argument(m,0).i, rtosc_argument(m,1).f);}},
    {"/amsynth/pitchWheel:i", rDoc("Set the pitch wheel"), 0,
        [](const char *m,rtosc::RtData &d){
            ((Synthesizer *)d.obj)->_voiceAllocationUnit->HandleMidiPitchWheel(rtosc_argument(m,0).f);}},
    {"/amsynth/pitchBendRange:i", rDoc("Set the pitch bend range"), 0,
        [](const char *m,rtosc::RtData &d){
            ((Synthesizer *)d.obj)->_voiceAllocationUnit->setPitchBendRangeSemitones(rtosc_argument(m,0).i);}},
	{"/amsynth/allSoundsOff", rDoc("Stop all sounds"), 0, rActionCb(_voiceAllocationUnit->HandleMidiAllSoundOff)},
	{"/amsynth/allNotesOff", rDoc("Release all notes"), 0, rActionCb(_voiceAllocationUnit->HandleMidiAllNotesOff)},
    {"/amsynth/sustainPedal:T", rDoc("Press the sustain pedal"), 0,
        [](const char *m,rtosc::RtData &d){
            ((Synthesizer *)d.obj)->_voiceAllocationUnit->HandleMidiSustainPedal(true);}},
    {"/amsynth/sustainPedal:F", rDoc("Release the sustain pedal"), 0,
        [](const char *m,rtosc::RtData &d){
            ((Synthesizer *)d.obj)->_voiceAllocationUnit->HandleMidiSustainPedal(false);}},
    {"/amsynth/maxVoices:i", rDoc("Set the maximum number of polyphonic voices"), 0,
        [](const char *m,rtosc::RtData &d){
            ((Synthesizer *)d.obj)->_voiceAllocationUnit->SetMaxVoices(rtosc_argument(m,0).i);}},
    {"/amsynth/aftertouchVelocity:if", rDoc("Change the velocity of a pressed key"), 0,
        [](const char *m,rtosc::RtData &d){
            ((Synthesizer *)d.obj)->_voiceAllocationUnit->HandleAftertouchVelocity(rtosc_argument(m,0).i, rtosc_argument(m,1).f);}},
    {"/amsynth/aftertouchPitch:if", rDoc("Change the pitch of a pressed key (absolute value)"), 0,
        [](const char *m,rtosc::RtData &d){
            ((Synthesizer *)d.obj)->_voiceAllocationUnit->HandleAftertouchPitch(rtosc_argument(m,0).i, rtosc_argument(m,1).f);}}
};
// end::ports[]
#undef rObject

#endif

Synthesizer::Synthesizer()
: _sampleRate(-1)
, _midiController(0)
, _presetController(0)
, _voiceAllocationUnit(0)
{
	Configuration &config = Configuration::get();

	_voiceAllocationUnit = new VoiceAllocationUnit;
	_voiceAllocationUnit->SetSampleRate((int) _sampleRate);
	_voiceAllocationUnit->SetMaxVoices(config.polyphony);
	_voiceAllocationUnit->setPitchBendRangeSemitones(config.pitch_bend_range);
	
	if (config.current_tuning_file != "default")
		_voiceAllocationUnit->loadScale(config.current_tuning_file.c_str());

	Preset::setIgnoredParameterNames(config.ignored_parameters);
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
						  const std::vector<amsynth_osc_event_t> &osc_in,
						  std::vector<amsynth_midi_cc_t> &midi_out,
						  float *audio_l, float *audio_r, unsigned audio_stride)
{
	if (_sampleRate < 0) {
		assert(0 == "sample rate has not been set");
		return;
	}

#ifdef WITH_RTOSC
	rtosc::RtData rtData = rtosc::RtData();
	for (std::vector<amsynth_osc_event_t>::const_iterator event = osc_in.begin(); event != osc_in.end(); event++) {
		ports.dispatch(event->buffer, rtData, true);
	}
#endif

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
