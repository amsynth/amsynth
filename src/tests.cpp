/*
 *  tests.cpp
 *
 *  Copyright (c) 2016 Nick Dowell
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

#include "controls.h"
#include "midi.h"
#include "MidiController.h"
#include "Synthesizer.h"
#include "VoiceBoard/Oscillator.h"
#include "VoiceBoard/LowPassFilter.h"

#include <cassert>
#include <cstdio>
#include <iostream>


void testMidiOutput()  {
    static float audioBuffer[64];

    Synthesizer *synth = new Synthesizer();
    synth->setSampleRate(44100);
    
    std::vector<amsynth_midi_event_t> midiIn;
    std::vector<amsynth_midi_cc_t> midiOut;
    
    synth->process(32, midiIn, midiOut, &audioBuffer[0], &audioBuffer[32]);
    
    Param param = kAmsynthParameter_ReverbWet;

    unsigned char cc = 2;
    synth->getMidiController()->setControllerForParameter(param, cc);
    
    for (unsigned char value = 0; value <= 127; value++) {
        unsigned char midi[4] = { MIDI_STATUS_CONTROLLER, cc, value };
        amsynth_midi_event_t event;
        event.length = 3;
        event.offset_frames = 0;
        event.buffer = midi;
        midiIn.clear();
        midiIn.push_back(event);
        
        midiOut.clear();
        synth->process(32, midiIn, midiOut, &audioBuffer[0], &audioBuffer[32]);

        int outputValue = (int)roundf(synth->getNormalizedParameterValue(param) * 127.0f);
        assert(outputValue == value || !"parameter value should be changed when a cc is processed");

        assert(midiOut.empty() || !"no midi output should be generated when a cc is processed");
    }
    
    midiIn.clear();
    midiOut.clear();
    synth->setNormalizedParameterValue(param, 0);
    synth->process(32, midiIn, midiOut, &audioBuffer[0], &audioBuffer[32]);
    assert(midiOut.size() == 1 || !"midi output should be generated when a parameter is changed");
    assert(midiOut[0].value == 0 || !"midi output value is incorrect");

    delete synth;
}

void testPresetIgnoredParameters() {
    Preset basePreset;
    basePreset.getParameter(0).setValue(1);
    Preset newPreset = basePreset;
    newPreset.getParameter(0).setValue(0);
    assert(!basePreset.isEqual(newPreset));
    Preset::setIgnoredParameterNames("amp_attack amp_decay");
    assert(basePreset.isEqual(newPreset));
    Preset::setIgnoredParameterNames("");
    assert(!basePreset.isEqual(newPreset));
}

size_t count(const char **strings) {
    size_t count;
    for (count = 0; strings[count]; count ++);
    return count;
}

void testPresetValueStrings() {
    assert(count(parameter_get_value_strings(kAmsynthParameter_Oscillator1Waveform)) == Oscillator::Waveform_Random + 1);
    assert(count(parameter_get_value_strings(kAmsynthParameter_Oscillator2Waveform)) == Oscillator::Waveform_Random + 1);
    assert(count(parameter_get_value_strings(kAmsynthParameter_KeyboardMode)) == KeyboardModeLegato + 1);
    assert(count(parameter_get_value_strings(kAmsynthParameter_FilterType)) == SynthFilter::FilterTypeCount);
    assert(count(parameter_get_value_strings(kAmsynthParameter_FilterSlope)) == SynthFilter::FilterSlope12 + 2);
    assert(count(parameter_get_value_strings(kAmsynthParameter_LFOOscillatorSelect)) == 3);
    assert(count(parameter_get_value_strings(kAmsynthParameter_PortamentoMode)) == PortamentoModeLegato + 1);
}

#define RUN_TEST(testFunction) do { printf("%s()... ", #testFunction); testFunction(); printf("OK\n"); } while (0)

int main(int argc, const char * argv[])  {
    RUN_TEST(testMidiOutput);
    RUN_TEST(testPresetIgnoredParameters);
    RUN_TEST(testPresetValueStrings);
    return 0;
}
