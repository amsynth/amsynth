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
#include "PresetController.h"

#include <cassert>
#include <cstdio>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <libgen.h>

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
    PresetController presetController;
    auto banks = std::string(dirname(dirname((char *)__FILE__))) + "/data/banks";
    DIR *dir = opendir(banks.c_str());

    struct dirent *entry;
    while ((entry = readdir(dir))) {
        if (!strstr(entry->d_name, "BriansBank")) {
            continue;
        }

        std::ostringstream path;
        path << banks << "/" << entry->d_name;
        int err = presetController.loadPresets(path.str().c_str());
        assert(err == 0);

        for (int i = 0; i < PresetController::kNumPresets; i ++) {
            presetController.selectPreset(i);
            std::ostringstream namestream;
            namestream << "/tmp/" << entry->d_name << "_" << i << ".json";
            std::string outname = namestream.str();
            std::cout << "Writing " << outname << std::endl;
            std::ofstream(outname, std::ios::out) << to_json(presetController.getCurrentPreset()).dump(2);
        }
    }
    closedir(dir);

    return 0;
}
