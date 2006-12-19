/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#ifndef _MIDICONTROLLER_H
#define _MIDICONTROLLER_H

#include "PresetController.h"
#include "drivers/MidiInterface.h"
#include "Parameter.h"
#include "Thread.h"

// there are 32 standard MIDI controllers
#define MAX_CC 128

/**
 * The MidiController is run as a thread which reads from the MIDI input device,
 * decodes the incoming messages and performs the appropriate actions on the
 * rest of the system.
 */

typedef unsigned char uchar;

class MidiEventHandler
{
public:
	virtual ~MidiEventHandler() {}
	
	virtual void HandleMidiNoteOn(int note, float velocity) {}
	virtual void HandleMidiNoteOff(int note, float velocity) {}
	virtual void HandleMidiPitchWheel(float value) {}
	virtual void HandleMidiAllSoundOff() {}
	virtual void HandleMidiAllNotesOff() {}
	virtual void HandleMidiSustainPedal(uchar value) {}
	virtual void HandleMidiProgramChange(uchar program) {}
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
	Parameter & getLastControllerParam() { return last_active_controller; };
	Parameter & getController( unsigned int controller_no );
	
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
};
#endif
