/*
 *  MidiController.h
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

#ifndef _MIDICONTROLLER_H
#define _MIDICONTROLLER_H

#include "PresetController.h"
#include "drivers/MidiInterface.h"
#include "Parameter.h"
#include "Thread.h"

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
	virtual void HandleMidiProgramChange(uchar /*program*/) {}
};

class MidiController : public MidiStreamReceiver
{
public:
	MidiController( Config & config );
	virtual ~MidiController();

	int		init();

	void	setPresetController	(PresetController & pc);
	void	SetMidiEventHandler(MidiEventHandler* h) { _handler = h; }
	
	virtual void HandleMidiData(unsigned char* bytes, unsigned numBytes);

	void	saveConfig ();

	void	setController		( unsigned int controller_no, Parameter &param );
	int     getControllerForParam(unsigned paramIdx);
	Parameter & getLastControllerParam() { return last_active_controller; };
	Parameter & getController( unsigned int controller_no );
	
	int		get_midi_channel	() { return channel; }
	void	set_midi_channel	( int ch );
	
	int     sendMidi_values		();

private:
    void dispatch_note(unsigned char ch,
		       unsigned char note, unsigned char vel);
    void controller_change(unsigned char controller, unsigned char value);
    void pitch_wheel_change(float val);

    PresetController *presetController;
	Config *config;
    unsigned char status, data, channel;
	Parameter last_active_controller;
	Parameter *midi_controllers[MAX_CC];
	MidiEventHandler* _handler;
	unsigned char _rpn_msb, _rpn_lsb;
};
#endif
