/*
 *  MidiController.h
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

#ifndef _MIDICONTROLLER_H
#define _MIDICONTROLLER_H

#include "PresetController.h"
#include "Parameter.h"
#include "types.h"


#define MAX_CC 128

typedef unsigned char uchar;

class MidiEventHandler
{
public:
	virtual ~MidiEventHandler() {}
	
	virtual void HandleMidiNoteOn(int /*note*/, float /*velocity*/) {}
	virtual void HandleMidiNoteOff(int /*note*/, float /*velocity*/) {}
	virtual void HandleMidiPitchWheel(float /*value*/) {}
	virtual void HandleMidiPitchWheelSensitivity(uchar semitones) {}
	virtual void HandleMidiAllSoundOff() {}
	virtual void HandleMidiAllNotesOff() {}
	virtual void HandleMidiSustainPedal(uchar /*value*/) {}
	virtual void HandleMidiPan(float left, float right) {}
};

class MidiController
{
public:
	MidiController();

	void	setPresetController	(PresetController & pc) { presetController = &pc; }
	void	SetMidiEventHandler(MidiEventHandler* h) { _handler = h; }
	
	void	HandleMidiData(const unsigned char *bytes, unsigned numBytes);

	void	clearControllerMap();
	void	loadControllerMap();

	int		getControllerForParameter(Param paramId);
	void	setControllerForParameter(Param paramId, int cc);

	Parameter & getLastControllerParam() { return last_active_controller; };

	void 	generateMidiOutput	(std::vector<amsynth_midi_cc_t> &);

	unsigned char assignedChannel = 0; // 0 denotes any channel

private:
	void dispatch_note(unsigned char ch,
		       unsigned char note, unsigned char vel);
    void controller_change(unsigned char controller, unsigned char value);
    void pitch_wheel_change(float val);

    void saveControllerMap();

    PresetController *presetController = nullptr;
    unsigned char status, data, channel;
	Parameter last_active_controller{"last_active_cc", (Param) -1, 0, 0, MAX_CC, 1};
	unsigned char _midi_cc_vals[MAX_CC];
	MidiEventHandler* _handler = nullptr;
	unsigned char _rpn_msb = 0xff;
	unsigned char _rpn_lsb = 0xff;

	int _cc_to_param_map[MAX_CC];
	int _param_to_cc_map[kAmsynthParameterCount];
};
#endif
