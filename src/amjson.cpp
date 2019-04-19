/*
 *  amjson.cpp
 *
 *  Copyright (c) 2019 Nick Dowell
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

#include "json.hpp"
#include "midi.h"
#include "PresetController.h"
#include "Synthesizer.h"

#include <cassert>
#include <cstdio>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <libgen.h>
#include <unistd.h>

static const char *osc_waveforms[] = {
    "sine",
    "pulse",
    "saw",
    "noise",
    "noise_sample_hold"
};

static const char *lfo_waveforms[] = {
    "sine",
    "square",
    "triangle",
    "noise",
    "noise_sample_hold",
    "saw_up",
    "saw_down"
};

static const char *keyboard_modes[] = {
    "poly",
    "mono",
    "legato"
};

static const char *filter_types[] = {
    "lowpass",
    "highpass",
    "bandpass",
    "notch",
    "bypass"
};

static const char *lfo_osc_modes[] = {
    "osc1+2",
    "osc1",
    "osc2"
};

static const char *portamento_modes[] = {
    "always",
    "legato"
};

static std::string named_value(Parameter &parameter, const char *values[]) {
    return values[(int)parameter.getValue()];
}

static nlohmann::json to_json(Preset &preset) {
    nlohmann::json json;
    json["name"] = preset.getName();

    for (int i = 0; i < kAmsynthParameterCount; i++) {
        auto parameter = preset.getParameter(i);
#define VALUE json[parameter.getName()]
        switch (parameter.GetId()) {
            case kAmsynthParameter_AmpEnvAttack:
            case kAmsynthParameter_AmpEnvDecay:
            case kAmsynthParameter_AmpEnvRelease:
            case kAmsynthParameter_FilterEnvAttack:
            case kAmsynthParameter_FilterEnvDecay:
            case kAmsynthParameter_FilterEnvRelease:
            case kAmsynthParameter_PortamentoTime:
                VALUE = parameter.getControlValue(); // 0.0005 - 15.6255
                break;

            case kAmsynthParameter_AmpEnvSustain:
            case kAmsynthParameter_FilterEnvSustain:
                VALUE = parameter.getControlValue(); // 0 - 1
                break;

            case kAmsynthParameter_Oscillator1Waveform:
                VALUE = named_value(parameter, osc_waveforms);
                break;

            case kAmsynthParameter_FilterResonance:
                VALUE = parameter.getControlValue(); // 0 - 0.97
                break;

            case kAmsynthParameter_FilterEnvAmount:
                VALUE = parameter.getControlValue(); // -16 - 16
                break;

            case kAmsynthParameter_FilterCutoff:
                VALUE = parameter.GetNormalisedValue(); // 0 - 1
                break;

            case kAmsynthParameter_Oscillator2Detune:
                VALUE = 1200.0 * log2(parameter.getControlValue()); // -100 - 100 cents
                break;

            case kAmsynthParameter_Oscillator2Waveform:
                VALUE = named_value(parameter, osc_waveforms);
                break;

            case kAmsynthParameter_MasterVolume:
                VALUE = parameter.getControlValue(); // 0 - 1
                break;

            case kAmsynthParameter_LFOFreq:
                VALUE = parameter.getControlValue(); // 0 - 56.25
                break;

            case kAmsynthParameter_LFOWaveform:
                VALUE = named_value(parameter, lfo_waveforms);
                break;

            case kAmsynthParameter_Oscillator2Octave:
                VALUE = parameter.getValue(); // -3 - 4
                break;

            case kAmsynthParameter_OscillatorMix:
                VALUE = parameter.getValue(); // -1 - 1
                break;

            case kAmsynthParameter_LFOToOscillators:
            case kAmsynthParameter_LFOToFilterCutoff:
            case kAmsynthParameter_LFOToAmp:
                VALUE = parameter.GetNormalisedValue();
                break;

            case kAmsynthParameter_OscillatorMixRingMod:
                VALUE = parameter.getControlValue();
                break;

            case kAmsynthParameter_Oscillator1Pulsewidth:
            case kAmsynthParameter_Oscillator2Pulsewidth:
                VALUE = parameter.GetNormalisedValue(); // 0 - 1
                break;

            case kAmsynthParameter_ReverbRoomsize:
            case kAmsynthParameter_ReverbDamp:
            case kAmsynthParameter_ReverbWet:
            case kAmsynthParameter_ReverbWidth:
                VALUE = parameter.GetNormalisedValue(); // 0 - 1
                break;

            case kAmsynthParameter_AmpDistortion:
                VALUE = parameter.GetNormalisedValue(); // 0 - 1
                break;

            case kAmsynthParameter_Oscillator2Sync:
                VALUE = parameter.GetNormalisedValue() > 0.5;
                break;

            case kAmsynthParameter_KeyboardMode:
                VALUE = named_value(parameter, keyboard_modes);
                break;

            case kAmsynthParameter_Oscillator2Pitch:
                VALUE = parameter.getControlValue(); // -12 - 12
                break;

            case kAmsynthParameter_FilterType:
                VALUE = named_value(parameter, filter_types);
                break;

            case kAmsynthParameter_FilterSlope:
                VALUE = parameter.getValue() < 0.5 ? 12 : 24;
                break;

            case kAmsynthParameter_LFOOscillatorSelect:
                VALUE = named_value(parameter, lfo_osc_modes);
                break;

            case kAmsynthParameter_FilterKeyTrackAmount:
                VALUE = parameter.GetNormalisedValue(); // 0 - 1
                break;

            case kAmsynthParameter_FilterKeyVelocityAmount:
                VALUE = parameter.GetNormalisedValue(); // 0 - 1
                break;

            case kAmsynthParameter_AmpVelocityAmount:
                VALUE = parameter.GetNormalisedValue(); // 0 - 1
                break;

            case kAmsynthParameter_PortamentoMode:
                VALUE = named_value(parameter, portamento_modes);
                break;

            case kAmsynthParameterCount:
                assert(!"kAmsynthParameterCount is not a valid parameter ID");
                break;
        }
    }

    return json;
}

int main(int argc, const char * argv[]) {
    auto banks = std::string(dirname(dirname((char *)__FILE__))) + "/data/banks";
    DIR *dir = opendir(banks.c_str());
    
    float *floatL = (float *)malloc(sizeof(float) * 44100);
    float *floatR = (float *)malloc(sizeof(float) * 44100);
    int16_t *shortBuffer = (int16_t *)malloc(sizeof(int16_t) * 44100 * 2);

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (!strstr(entry->d_name, "BriansBank")) {
            continue;
        }

        std::ostringstream path;
        path << banks << "/" << entry->d_name;
        
        Synthesizer synth;
        synth.setSampleRate(44100);
        synth.loadBank(path.str().c_str());

        for (int i = 0; i < PresetController::kNumPresets; i++) {
            synth.setPresetNumber(i);
            
            // Output JSON
            
            std::ostringstream namestream;
            namestream << "/tmp/" << entry->d_name << "_" << i << ".json";
            std::string outname = namestream.str();
            std::cout << "Writing " << outname << std::endl;
            std::ofstream(outname, std::ios::out) << to_json(synth.getPresetController()->getCurrentPreset()).dump(2);
            
            // Generate audio sample
            
            std::vector<amsynth_midi_event_t> midiIn;
            std::vector<amsynth_midi_cc_t> midiOut;
            
            unsigned char midi[4] = { MIDI_STATUS_NOTE_ON, 69, 127 };
            amsynth_midi_event_t e = { 0, 3, midi };
            midiIn.clear();
            midiIn.push_back(e);

            synth.process(44100, midiIn, midiOut, floatL, floatR);
            
            for (int j = 0; j < 44100; j++) {
                shortBuffer[j * 2 + 0] = (int16_t)(floatL[j] * 32767.f);
                shortBuffer[j * 2 + 1] = (int16_t)(floatR[j] * 32767.f);
            }
            
            std::ostringstream wavnamestream;
            wavnamestream << "/tmp/" << entry->d_name << "_" << i << ".wav";
            std::cout << "Generating " << wavnamestream.str() << std::endl;
            
            int wav = open(wavnamestream.str().c_str(), O_RDWR | O_CREAT | O_TRUNC, 0664);
            
#define write_int(fd, value) do { int tmp = value; write(fd, &tmp, sizeof(tmp)); } while (0)
#define write_short(fd, value) do { uint16_t tmp = value; write(fd, &tmp, sizeof(tmp)); } while (0)
            
            write(wav, "RIFF", 4);
            write_int(wav, 18 + 176400);
            write(wav, "WAVE", 4);
            write(wav, "fmt ", 4);
            write_int(wav, 16); // Should be 16 for PCM
            write_short(wav, 1); // PCM = 1 (i.e. Linear quantization)
            write_short(wav, 2); // Number of channels
            write_int(wav, 44100); // Sample rate
            write_int(wav, 176400); // SampleRate * NumChannels * BitsPerSample/8
            write_short(wav, 4); // NumChannels * BitsPerSample/8
            write_short(wav, 16); // Number of bits per sample
            write(wav, "data", 4);
            write_int(wav, 176400); // Number of bytes that follow
            write(wav, shortBuffer, 176400);
            close(wav);
        }
    }
    closedir(dir);

    return 0;
}
