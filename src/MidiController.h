/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#ifndef _MIDICONTROLLER_H
#define _MIDICONTROLLER_H

#include "PresetController.h"
#include "drivers/MidiInterface.h"
#include "VoiceAllocationUnit.h"
#include "Parameter.h"

// there are 32 standard MIDI controllers
#define MAX_CC 128

/**
 * The MidiController is run as a thread which reads from the MIDI input device,
 * decodes the incoming messages and performs the appropriate actions on the
 * rest of the system.
 */
class MidiController {
  public:
    MidiController( Config & config );
    ~MidiController();
	/**
	 * @param pc The PresetController for the system.
	 */
    void setPresetController(PresetController & pc);
	/**
	 * @param vau The VoiceAllocationUnit for the system.
	 */
    void setVAU(VoiceAllocationUnit & vau);
	/**
	 * @param config The global Config object for the system.
	 */
	void saveConfig();
	/**
	 * Start execution of the MidiController. This function never returns (until
	 * execution is stop()ped or an error occurs).
	 */
    void run();
	/**
	 * Stop the execution of the MidiController.
	 */
    void stop();
	void setController( int controller_no, Parameter &param );
	Parameter & getLastControllerParam()
	{ return last_active_controller; };
	Parameter & getController( int controller_no );
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
    char running;
    int bytes_read;
    unsigned char *buffer;
    unsigned char status, data, channel, byte;
	Parameter last_active_controller;
	Parameter *midi_controllers[MAX_CC];
};
#endif
