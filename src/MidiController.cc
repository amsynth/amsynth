/*
 *  MidiController.cc
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

#include "MidiController.h"

#include "drivers/MidiDriver.h"
#include "midi.h"

#include <assert.h>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>


MidiController::MidiController( Config & config )
:	last_active_controller ("last_active_cc", (Param) -1, 0, 0, MAX_CC, 1)
,	_handler(NULL)
,	_rpn_msb(0xff)
,	_rpn_lsb(0xff)
,	_config_needs_save(false)
,	_midiDriver(NULL)
{
	this->config = &config;
	presetController = 0;
	channel = config.midi_channel;
	loadControllerMap();
}

MidiController::~MidiController()
{
	if (_config_needs_save)
		saveControllerMap();
}

void
MidiController::HandleMidiData(const unsigned char* bytes, unsigned numBytes)
{
    for (unsigned i=0; i<numBytes; i++)
	{
		const unsigned char byte = bytes[i];
		
		if (byte & 0x80) {	// then byte is a status byte
			if (byte < 0xf0) {	// dont deal with system messages
			status = byte;
			channel = (byte & 0x0f);
			data = 0xff;
			}
			continue;
		}
		// now we have at least one data byte

		if (config->midi_channel && ((int) channel != config->midi_channel-1)) break;

		switch (status & 0xf0)
		{
		case MIDI_STATUS_NOTE_OFF:
			// N.B. many devices send a 'note on' event with 0 velocity
			// rather than a distinct 'note off' event.
			if (data == 0xff) {
				data = byte;
				break;
			}
			dispatch_note( channel, data, 0 );
			data = 0xff;
			break;
	
		case MIDI_STATUS_NOTE_ON:
			if (data == 0xff) {
				data = byte;
				break;
			}
			dispatch_note(channel, data, byte);
			data = 0xff;
			break;
	
		case MIDI_STATUS_NOTE_PRESSURE:
			if (data == 0xff) {
				data = byte;
				break;
			}
			data = 0xff;
			break;

		case MIDI_STATUS_CONTROLLER:
			if (data == 0xFF) {
				data = byte;
				break;
			}
			controller_change(data, byte);
			data = 0xFF;
			break;

		case MIDI_STATUS_PROGRAM_CHANGE:
			if (presetController->getCurrPresetNumber() != byte) {
				if (_handler) _handler->HandleMidiAllSoundOff();
				presetController->selectPreset((int) byte);
			}
			data = 0xff;
			break;
	
		case MIDI_STATUS_CHANNEL_PRESSURE:
			data = 0xff;
			break;
	
		case MIDI_STATUS_PITCH_WHEEL:
			// 2 data bytes give a 14 bit value, least significant 7 bits
			// first
			if (data == 0xFF) {
				data = byte;
				break;
			}
			int bend; bend = (int) ((data & 0x7F) | ((byte & 0x7F) << 7));
			float fbend; fbend = (float) (bend - 0x2000) / (float) (0x2000);
			pitch_wheel_change(fbend);
			data = 0xFF;
			break;
	
		default:
#ifdef _DEBUG
			std::cout << "<MidiController> unknown status :" << std::hex <<
			(int) status << "for data byte: " << std::hex << (int) byte <<
			std::endl;
#endif
			break;
		}
    }
}

void
MidiController::pitch_wheel_change(float val)
{
	if (_handler) _handler->HandleMidiPitchWheel(val);
}

void
MidiController::dispatch_note(unsigned char, unsigned char note, unsigned char vel)
{
	static const float scale = 1.f/127.f;
    if (!_handler) return;
	if (vel) _handler->HandleMidiNoteOn((int) note, (float)vel * scale);
	else     _handler->HandleMidiNoteOff((int) note, (float)vel * scale);
}

void
MidiController::controller_change(unsigned char cc, unsigned char value)
{
	if (!_handler || !presetController)
		return;

	switch (cc) {
		case MIDI_CC_BANK_SELECT_LSB:
		case MIDI_CC_BANK_SELECT_MSB:
			break;
		case MIDI_CC_PAN_MSB: {
			// http://www.midi.org/techspecs/rp36.php
			// the effective range for CC#10 is modified to be 1 to 127, and values 0 and 1 both pan hard left
			float scaled = (value < 1 ? 0 : value - 1) / 126.0;
			_handler->HandleMidiPan(cos(M_PI_2 * scaled), sin(M_PI_2 * scaled));
		}
			break;
		case MIDI_CC_SUSTAIN_PEDAL:
			_handler->HandleMidiSustainPedal(value);
			break;
		case MIDI_CC_DATA_ENTRY_MSB:
			if (_rpn_msb == 0x00 && _rpn_lsb == 0x00)
				_handler->HandleMidiPitchWheelSensitivity(value);
			break;
		case MIDI_CC_PORTAMENTO:
		case MIDI_CC_SOSTENUTO:
		case MIDI_CC_NRPN_LSB:
		case MIDI_CC_NRPN_MSB:
			break;
		case MIDI_CC_RPN_LSB:
			_rpn_lsb = value;
			break;
		case MIDI_CC_RPN_MSB:
			_rpn_msb = value;
			break;
		case MIDI_CC_ALL_SOUND_OFF:
			if (value == 0)
				_handler->HandleMidiAllSoundOff();
			break;
		case MIDI_CC_RESET_ALL_CONTROLLERS:
			// http://www.midi.org/techspecs/rp15.php
			_handler->HandleMidiPitchWheel(0);
			break;
		case MIDI_CC_LOCAL_CONTROL:
			break;
		case MIDI_CC_ALL_NOTES_OFF:
			if (value == 0)
				_handler->HandleMidiAllNotesOff();
			break;
		case MIDI_CC_OMNI_MODE_OFF:
		case MIDI_CC_OMNI_MODE_ON:
		case MIDI_CC_MONO_MODE_ON:
		case MIDI_CC_POLY_MODE_ON:
			_handler->HandleMidiAllNotesOff();
		case MIDI_CC_MODULATION_WHEEL:
		default:
			if (last_active_controller.getValue() != cc)
				last_active_controller.setValue(cc);
			int paramId = _cc_to_param_map[cc];
			if (paramId >= 0)
				presetController->getCurrentPreset().getParameter(paramId).SetNormalisedValue(value / 127.0f);
			_midi_cc_vals[cc] = value;
			break;
	}
	return;
}

void
MidiController::timer_callback()
{
	if (_config_needs_save)
		saveControllerMap();
}

void
MidiController::clearControllerMap()
{
	for (size_t i = 0; i < MAX_CC; i++) {
		_cc_to_param_map[i] = -1;
		_midi_cc_vals[i] = 0;
	}
	for (size_t i = 0; i < kAmsynthParameterCount; i++)
		_param_to_cc_map[i] = -1;

	// these are the defaults from /usr/share/amsynth/Controllersrc
	_cc_to_param_map[1] = kAmsynthParameter_LFOToOscillators;
	_param_to_cc_map[kAmsynthParameter_LFOToOscillators] = 1;
	_cc_to_param_map[7] = kAmsynthParameter_MasterVolume;
	_param_to_cc_map[kAmsynthParameter_MasterVolume] = 7;

	_config_needs_save = false;
}

void
MidiController::loadControllerMap()
{
	clearControllerMap();

	std::string fname(getenv("HOME"));
	fname += "/.amSynthControllersrc";
	std::ifstream file(fname.c_str(), std::ios::out);
	std::string name;
	file >> name;
	for (int cc = 0; cc < MAX_CC && file.good(); cc++, file >> name) {
		int paramId = parameter_index_from_name(name.c_str());
		_cc_to_param_map[cc] = paramId;
		_param_to_cc_map[paramId] = cc;
	}
	file.close();

	_config_needs_save = false;
}

void
MidiController::saveControllerMap()
{
	std::string fname(getenv("HOME"));
	fname += "/.amSynthControllersrc";
	std::ofstream file(fname.c_str(), std::ios::out);
	if (file.bad())
		return;
  	for (unsigned char cc = 0; cc < MAX_CC; cc++) {
		int paramId = _cc_to_param_map[cc];
		const char *name = parameter_name_from_index(paramId);
		file << (name ? name : "null") << std::endl;
	}
	file.close();

	_config_needs_save = false;
}

int
MidiController::getControllerForParameter(int paramId)
{
	assert(0 <= paramId && paramId < kAmsynthParameterCount);
	return _param_to_cc_map[paramId];
}

void
MidiController::setControllerForParameter(int paramId, int cc)
{
	assert(paramId < kAmsynthParameterCount && cc < MAX_CC);

	if (0 <= paramId) {
		int old_cc = _param_to_cc_map[paramId];
		if (0 <= old_cc)
			_cc_to_param_map[old_cc] = -1;
		_param_to_cc_map[paramId] = cc;
	}

	if (0 <= cc) {
		int old_param = _cc_to_param_map[cc];
		if (0 <= old_param)
			_param_to_cc_map[old_param] = -1;
		_cc_to_param_map[cc] = paramId;
	}

	_config_needs_save = true;
}

void
MidiController::set_midi_channel	( int ch )
{
	if (ch)	_handler->HandleMidiAllSoundOff();
	config->midi_channel = ch;
}

int
MidiController::sendMidi_values       ()
{
	send_changes(true);
	return 0;
}

void
MidiController::send_changes(bool force)
{
	if (!_midiDriver)
		return;
	for (size_t paramId = 0; paramId < kAmsynthParameterCount; paramId++) {
		int cc = _param_to_cc_map[paramId];
		if (0 <= cc && cc < MAX_CC) {
			Parameter &parameter = presetController->getCurrentPreset().getParameter(paramId);
			unsigned char value = parameter.GetNormalisedValue() * 127.0;
			if (_midi_cc_vals[cc] != value || force) {
				_midi_cc_vals[cc] = value;
				_midiDriver->write_cc(channel, cc, value);
			}
		}
	}
}
