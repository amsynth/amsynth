/* Synth--
 * (c) 2001 Nick Dowell
 */

#ifndef _EVENTCONTROLLER_H
#define _EVENTCONTROLLER_H

#include "MidiController.h"
#include "PresetController.h"

class EventController {
  public:
    EventController();
    ~EventController();
    setMidiController(&MidiController mc);
    setPresetController(&PresetController pc);
    &PresetController getPresetController();
  private:
};
#endif
