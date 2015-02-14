/*
 *  Synthesizer.cpp
 *
 *  Copyright (c) 2014 Nick Dowell
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

#ifndef __amsynth__Synthesizer__
#define __amsynth__Synthesizer__

#include "types.h"
#include "controls.h"

#include <vector>

class Config;
class MidiController;
class PresetController;
class VoiceAllocationUnit;

class Synthesizer
{
public:
    
    Synthesizer(Config *config = 0);
    ~Synthesizer();
    
    void loadBank(const char *filename);
    void saveBank(const char *filename);
    
    const char *getPresetName(int presetNumber);

    int getPresetNumber();
    void setPresetNumber(int number);

    float getParameterValue(Param parameter);
    void setParameterValue(Param parameter, float value);

	int getPitchBendRangeSemitones();
	void setPitchBendRangeSemitones(int value);

	int getMaxNumVoices();
	void setMaxNumVoices(int value);

	int loadTuningKeymap(const char *filename);
	int loadTuningScale(const char *filename);
	void defaultTuning();

	void setSampleRate(int sampleRate);

    void process(unsigned nframes,
                 std::vector<amsynth_midi_event_t> &midi_in,
                 float *audio_l, float *audio_r,
                 unsigned audio_stride = 1);
    
    MidiController *getMidiController() DEPRECATED { return _midiController; };
    PresetController *getPresetController() DEPRECATED { return _presetController; }
    
private:

    double _sampleRate;
    MidiController *_midiController;
    PresetController *_presetController;
    VoiceAllocationUnit *_voiceAllocationUnit;
};

#endif /* defined(__amsynth__Synthesizer__) */
