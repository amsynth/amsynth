/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#ifndef _MIDICONTROLLER_H
#define _MIDICONTROLLER_H

#include "PresetController.h"
#include "drivers/MidiInterface.h"
#include "VoiceAllocationUnit.h"

class MidiController {
  public:
    MidiController();
    ~MidiController();
    //  void setEventController( EventController &ev );
    void setPresetController(PresetController & pc);
    void setVAU(VoiceAllocationUnit & vau);
	void setConfig(Config & config){
		this->config = &config;
	};
    void run();
    void stop();
  private:
    void doMidi();
    void dispatch_note(unsigned char ch,
		       unsigned char note, unsigned char vel);
    void controller_change(unsigned char controller, unsigned char value);
    void pitch_wheel_change(float val);
    //  EventController _ev;
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
