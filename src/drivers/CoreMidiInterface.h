/*
 *  CoreMidiInterface.h
 *  amsynth
 */

#ifndef _CoreMidiInterface_h
#define _CoreMidiInterface_h

#include "AudioOutput.h"
#include "MidiInterface.h"

GenericOutput* CreateCoreAudioOutput();
MidiInterface* CreateCoreMidiInterface();

#endif
