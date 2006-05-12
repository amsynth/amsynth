/*
 *  CoreAudio.h
 *  amsynth
 */

#ifndef _CoreAudio_h
#define _CoreAudio_h

#include "AudioOutput.h"
#include "MidiInterface.h"

GenericOutput* CreateCoreAudioOutput();
MidiInterface* CreateCoreMidiInterface();

#endif
