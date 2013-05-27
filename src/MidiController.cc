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

#include "midi.h"

#include <assert.h>
#include <cstdlib>
#include <fstream>
#include <iostream>

using namespace std;

MidiController::MidiController( Config & config )
:	last_active_controller ("last_active_cc", (Param) -1, 0, 0, MAX_CC, 1)
,	_handler(NULL)
,	_rpn_msb(0xff)
,	_rpn_lsb(0xff)
,	_config_needs_save(false)
{
	this->config = &config;
	presetController = 0;
	channel = config.midi_channel;
	for( int i=0; i<MAX_CC; i++ ) midi_controllers[i] = 0;
}

MidiController::~MidiController()
{
	if (_config_needs_save)
		saveConfig();
}

void
MidiController::setPresetController(PresetController & pc)
{
	presetController = &pc;
	
	for(int i=0; i<MAX_CC; i++)
		midi_controllers[i] = &(presetController->getCurrentPreset().
				getParameter("null"));

	midi_controllers[1] = &(presetController->getCurrentPreset().
			getParameter("freq_mod_amount"));
	midi_controllers[7] = &(presetController->getCurrentPreset().
			getParameter("master_vol"));
	
	// load controller mapping config. from file	
	string fname(getenv("HOME"));
	fname += "/.amSynthControllersrc";
	ifstream file(fname.c_str(), ios::out);
	int i=0;
	string buffer;
	if (file.bad())	return;
  
	file >> buffer;
	while( file.good() )
	{
		midi_controllers[i++] = &(presetController->getCurrentPreset().
				getParameter( buffer ));
		file >> buffer;
	}
	file.close();
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
			cout << "<MidiController> unknown status :" << hex <<
			(int) status << "for data byte: " << hex << (int) byte <<
			endl;
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
	switch (cc) {
		case MIDI_CC_BANK_SELECT_LSB:
		case MIDI_CC_BANK_SELECT_MSB:
			break;
		case MIDI_CC_SUSTAIN_PEDAL:
			if (_handler)
				_handler->HandleMidiSustainPedal(value);
			break;
		case MIDI_CC_DATA_ENTRY_MSB:
			if (_handler && _rpn_msb == 0x00 && _rpn_lsb == 0x00)
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
			if (_handler && value == 0)
				_handler->HandleMidiAllSoundOff();
			break;
		case MIDI_CC_RESET_ALL_CONTROLLERS:
		case MIDI_CC_LOCAL_CONTROL:
			break;
		case MIDI_CC_ALL_NOTES_OFF:
			if (_handler && value == 0)
				_handler->HandleMidiAllNotesOff();
			break;
		case MIDI_CC_OMNI_MODE_OFF:
		case MIDI_CC_OMNI_MODE_ON:
		case MIDI_CC_MONO_MODE_ON:
		case MIDI_CC_POLY_MODE_ON:
			_handler->HandleMidiAllNotesOff();
		case MIDI_CC_MODULATION_WHEEL:
		default:
			if (last_active_controller.getValue() != cc) {
				last_active_controller.setValue(cc);
			}
			getController(cc).SetNormalisedValue(value / 127.0f);
			break;
	}
	return;
}

void
MidiController::setController( unsigned int controller_no, Parameter & param )
{
	if(controller_no<MAX_CC)
		midi_controllers[controller_no] = &param;
	_config_needs_save = true;
}

Parameter&
MidiController::getController( unsigned int idx )
{
    assert(idx < MAX_CC);
    return (idx < MAX_CC) ? *midi_controllers[idx] : presetController->getCurrentPreset().getParameter("null");
}

int
MidiController::getControllerForParam(unsigned paramIdx)
{
	for (unsigned int i=0; i<MAX_CC; i++) {
		if (midi_controllers[i] &&
			midi_controllers[i]->GetId() == i) {
			return i;
		}
	}
	return -1;
}

void
MidiController::timer_callback()
{
	if (_config_needs_save)
		saveConfig();
}

void
MidiController::saveConfig()
{
	string fname(getenv("HOME"));
	fname += "/.amSynthControllersrc";
	ofstream file(fname.c_str(), ios::out);
	if (file.bad())	return;
  
	for(int i=0; i<MAX_CC; i++){
		file << midi_controllers[i]->getName() << endl;
	}
	file.close();

	_config_needs_save = false;
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
	if (!_midiIface)
	{
		std::cerr << "error: cannot send midi values, no midi output interface present\n";
		return -1;
	}

      for (unsigned i = 0; i < MAX_CC; i++)
      {
              if (midi_controllers[i]->getName() != "null")
              {
      
                      float fval=midi_controllers[i]->getValue();
                      int midiValue=(int)(127*(fval-midi_controllers[i]->getMin())/(midi_controllers[i]->getMax()-midi_controllers[i]->getMin()));
#ifdef _DEBUG
                      cout << "PresetController::midiSendPresets :- parameter name="
                      << midi_controllers[i]->getName() << " value= "
                      << midi_controllers[i]->getValue() 
                      << " midiValue = " << midiValue << endl;
#endif
                      if (_midiIface) _midiIface->write_cc(0,i,midiValue);
              }
      }
      return 0;
}



