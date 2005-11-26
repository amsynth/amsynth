/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#ifndef _MIDICONTROLLER_H
#define _MIDICONTROLLER_H

#include "PresetController.h"
#include "drivers/MidiInterface.h"
#include "VoiceAllocationUnit.h"
#include "Parameter.h"
#include "Thread.h"

// there are 32 standard MIDI controllers
#define MAX_CC 128

/**
 * The MidiController is run as a thread which reads from the MIDI input device,
 * decodes the incoming messages and performs the appropriate actions on the
 * rest of the system.
 */
class MidiController : public Thread
{
public:
	MidiController( Config & config );
	virtual ~MidiController();

	int	init			( );

	void	Stop ();

	void	setPresetController	(PresetController & pc);
	void	setVAU			(VoiceAllocationUnit & vau);

	void	saveConfig ();

	void	setController		( int controller_no, Parameter &param );
	Parameter & getLastControllerParam() { return last_active_controller; };
	Parameter & getController( int controller_no );
	
	void	set_midi_channel	( int ch );
	int     sendMidi_values		();

protected:
	void	ThreadAction ();

private:
    void doMidi();
    void dispatch_note(unsigned char ch,
		       unsigned char note, unsigned char vel);
    void controller_change(unsigned char controller, unsigned char value);
    void pitch_wheel_change(float val);
    VoiceAllocationUnit *_va;
    MidiInterface midi;
    PresetController *presetController;
	Config *config;
    int bytes_read;
    unsigned char *buffer;
    unsigned char status, data, channel, byte;
	Parameter last_active_controller;
	Parameter *midi_controllers[MAX_CC];
};
#endif
