/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#ifndef _MIDICONTROLLER_H
#define _MIDICONTROLLER_H

#include "PresetController.h"
#include "drivers/MidiInterface.h"
#include "VoiceAllocationUnit.h"

/**
 * The MidiController is run as a thread which reads from the MIDI input device,
 * decodes the incoming messages and performs the appropriate actions on the
 * rest of the system.
 */
class MidiController {
  public:
    MidiController();
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
	void setConfig(Config & config){
		this->config = &config;
	};
	/**
	 * Start execution of the MidiController. This function never returns (until
	 * execution is stop()ped or an error occurs).
	 */
    void run();
	/**
	 * Stop the execution of the MidiController.
	 */
    void stop();
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
};
#endif
