/*
 *  Preset.cpp
 *
 *  Copyright (c) 2001-2020 Nick Dowell
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

#include "Preset.h"

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "controls.h"
#include "VoiceBoard/LowPassFilter.h"

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <cstdio>
#include <map>
#include <iterator>

#include "gettext.h"
#define _(string) gettext (string)

#ifdef _DEBUG
#include <iostream>
using namespace std;
#endif


static Parameter TimeParameter (const std::string name, Param id)
{
    return Parameter (name, id, 0, 0, 2.5f, 0, Parameter::Law::kPower, 3, 0.0005f, "s");
}

Preset::Preset			(const std::string name)
:	mName (name)
{
	//										name					id					def min max inc		ControlType			base offset label
	mParameters.push_back (TimeParameter	("amp_attack",			kAmsynthParameter_AmpEnvAttack));
	mParameters.push_back (TimeParameter	("amp_decay",			kAmsynthParameter_AmpEnvDecay));
    mParameters.push_back (Parameter		("amp_sustain",			kAmsynthParameter_AmpEnvSustain,		1));
    mParameters.push_back (TimeParameter	("amp_release",			kAmsynthParameter_AmpEnvRelease));
    mParameters.push_back (Parameter		("osc1_waveform",		kAmsynthParameter_Oscillator1Waveform,		2, 0, 4, 1));
    mParameters.push_back (TimeParameter	("filter_attack",		kAmsynthParameter_FilterEnvAttack));
    mParameters.push_back (TimeParameter	("filter_decay",		kAmsynthParameter_FilterEnvDecay));
    mParameters.push_back (Parameter		("filter_sustain",		kAmsynthParameter_FilterEnvSustain,		1));
    mParameters.push_back (TimeParameter	("filter_release",		kAmsynthParameter_FilterEnvRelease));
    mParameters.push_back (Parameter		("filter_resonance",	kAmsynthParameter_FilterResonance,	0, 0, 0.97f));
	mParameters.push_back (Parameter		("filter_env_amount",	kAmsynthParameter_FilterEnvAmount,	0, -16, 16));
    mParameters.push_back (Parameter		("filter_cutoff",		kAmsynthParameter_FilterCutoff,		1.5, -0.5, 1.5, 0, Parameter::Law::kExponential, 16, 0));
    mParameters.push_back (Parameter		("osc2_detune",			kAmsynthParameter_Oscillator2Detune,		0, -1, 1, 0, Parameter::Law::kExponential, 1.25f, 0));
    mParameters.push_back (Parameter		("osc2_waveform",		kAmsynthParameter_Oscillator2Waveform,		2, 0, 4, 1));
    mParameters.push_back (Parameter		("master_vol",			kAmsynthParameter_MasterVolume,			0.67f, 0, 1, 0, Parameter::Law::kPower, 2, 0));
    mParameters.push_back (Parameter		("lfo_freq",			kAmsynthParameter_LFOFreq,			0, 0, 7.5, 0, Parameter::Law::kPower, 2, 0,	"Hz"));
    mParameters.push_back (Parameter		("lfo_waveform",		kAmsynthParameter_LFOWaveform,		0, 0, 6, 1));
    mParameters.push_back (Parameter		("osc2_range",			kAmsynthParameter_Oscillator2Octave,		0, -3, 4, 1, Parameter::Law::kExponential, 2, 0));
	mParameters.push_back (Parameter		("osc_mix",				kAmsynthParameter_OscillatorMix,			0, -1, 1));
    mParameters.push_back (Parameter		("freq_mod_amount",		kAmsynthParameter_LFOToOscillators,		0, 0, 1.25992105f, 0, Parameter::Law::kPower, 3, -1));
	mParameters.push_back (Parameter		("filter_mod_amount",	kAmsynthParameter_LFOToFilterCutoff,	-1, -1, 1));
	mParameters.push_back (Parameter		("amp_mod_amount",		kAmsynthParameter_LFOToAmp,		-1, -1, 1));
	mParameters.push_back (Parameter		("osc_mix_mode",		kAmsynthParameter_OscillatorMixRingMod,		0, 0, 1, 0));
	mParameters.push_back (Parameter		("osc1_pulsewidth",		kAmsynthParameter_Oscillator1Pulsewidth,	1));
	mParameters.push_back (Parameter		("osc2_pulsewidth",		kAmsynthParameter_Oscillator2Pulsewidth,	1));
	mParameters.push_back (Parameter		("reverb_roomsize",		kAmsynthParameter_ReverbRoomsize));
	mParameters.push_back (Parameter		("reverb_damp",			kAmsynthParameter_ReverbDamp));
	mParameters.push_back (Parameter		("reverb_wet",			kAmsynthParameter_ReverbWet));
	mParameters.push_back (Parameter		("reverb_width",		kAmsynthParameter_ReverbWidth,		1));
	mParameters.push_back (Parameter		("distortion_crunch",	kAmsynthParameter_AmpDistortion,	0, 0, 0.9f));
	mParameters.push_back (Parameter		("osc2_sync",			kAmsynthParameter_Oscillator2Sync,			0, 0, 1, 1));
	mParameters.push_back (Parameter		("portamento_time",		kAmsynthParameter_PortamentoTime, 0.0f, 0.0f, 1.0f));
	mParameters.push_back (Parameter		("keyboard_mode",		kAmsynthParameter_KeyboardMode, KeyboardModePoly, 0, KeyboardModeLegato, 1));
	mParameters.push_back (Parameter		("osc2_pitch",			kAmsynthParameter_Oscillator2Pitch, 0, -12, +12, 1));
	mParameters.push_back (Parameter		("filter_type",         kAmsynthParameter_FilterType, (int)SynthFilter::Type::kLowPass, (int)SynthFilter::Type::kLowPass, (int)SynthFilter::Type::kBypass, 1));
	mParameters.push_back (Parameter		("filter_slope",        kAmsynthParameter_FilterSlope, (int)SynthFilter::Slope::k24, (int)SynthFilter::Slope::k12, (int)SynthFilter::Slope::k24, 1));
	mParameters.push_back (Parameter		("freq_mod_osc",		kAmsynthParameter_LFOOscillatorSelect, 0, 0, 2, 1));
	mParameters.push_back (Parameter		("filter_kbd_track",    kAmsynthParameter_FilterKeyTrackAmount, 1));
	mParameters.push_back (Parameter		("filter_vel_sens",		kAmsynthParameter_FilterKeyVelocityAmount, 1));
	mParameters.push_back (Parameter		("amp_vel_sens",		kAmsynthParameter_AmpVelocityAmount, 1));
	mParameters.push_back (Parameter		("portamento_mode",		kAmsynthParameter_PortamentoMode, PortamentoModeAlways));
}

Preset&
Preset::operator =		(const Preset &rhs)
{
    for (unsigned i=0; i<rhs.ParameterCount(); i++) {
		if (shouldIgnoreParameter(i))
			continue;
		getParameter(i).setValue(rhs.getParameter(i).getValue());
    }
    setName(rhs.getName());
    return *this;
}

bool
Preset::isEqual(const Preset &rhs)
{
	for (unsigned i = 0; i < mParameters.size(); i++) {
		if (shouldIgnoreParameter(i))
			continue;
		if (getParameter(i).getValue() != rhs.getParameter(i).getValue()) {
			return false;
		}
	}
	return getName() == rhs.getName();
}

Parameter & 
Preset::getParameter(const std::string name)
{
	typedef std::map<std::string, size_t> name_map_t;
	static name_map_t name_map;
	if (name_map.empty()) {
		for (size_t i = 0; i < mParameters.size(); i++) {
			name_map[mParameters[i].getName()] = i;
		}
	}
	name_map_t::iterator it = name_map.find(name);
	if (it == name_map.end())
		return nullparam;
	return getParameter((int) it->second);
}

void
Preset::randomise()
{
	for (auto &parameter : mParameters) {
		if (parameter.getId() != kAmsynthParameter_MasterVolume) {
			parameter.randomise();
		}
	}
}

void
Preset::AddListenerToAll	(UpdateListener* ul)
{
	for (auto &parameter : mParameters) {
		parameter.addUpdateListener(ul);
	}
}

void
Preset::toString(std::stringstream &stream)
{
	stream << "amSynth1.0preset" << std::endl;
	stream << "<preset> " << "<name> " << getName() << std::endl;
	for (unsigned n = 0; n < ParameterCount(); n++) {
		stream << "<parameter> " << getParameter(n).getName() << " " << getParameter(n).getValue() << std::endl;
	}
}

bool
Preset::fromString(const std::string &str)
{
	std::stringstream stream (str);

	std::string buffer;
  
	stream >> buffer;
  
	if (buffer != "amSynth1.0preset") return false;
  
	stream >> buffer;
	if (buffer == "<preset>") {
		stream >> buffer;
		
		//get the preset's name
		stream >> buffer;
		std::string presetName;
		presetName += buffer;
		stream >> buffer;
		while (buffer != "<parameter>") {
			presetName += " ";
			presetName += buffer;
			stream >> buffer;
		}
		setName(presetName); 
		
		//get the parameters
		while (buffer == "<parameter>") {
			std::string name;
			stream >> buffer;
			name = buffer;
			stream >> buffer;
			if (name!="unused")
				getParameter(name).setValue(Parameter::valueFromString(buffer));
			stream >> buffer;
		}
	};
	return true;
}

void get_parameter_properties(int parameter_index, double *minimum, double *maximum, double *default_value, double *step_size)
{
    Preset preset;
    Parameter &parameter = preset.getParameter(parameter_index);
    
    if (minimum) {
        *minimum = parameter.getMin();
    }
    if (maximum) {
        *maximum = parameter.getMax();
    }
    if (default_value) {
        *default_value = parameter.getValue();
    }
    if (step_size) {
        *step_size = parameter.getStep();
    }
}

/* this implements the C API in controls.h */

static const Preset &_get_preset()
{
	static const Preset preset;
	return preset;
}

const char *parameter_name_from_index (int param_index)
{
	const Preset &_preset = _get_preset();
	if (param_index < 0 || param_index >= (int)_preset.ParameterCount())
		return nullptr;
	static std::vector<std::string> names;
	if (names.empty())
		names.resize(_preset.ParameterCount());
	if (names[param_index].empty())
		names[param_index] = _preset.getParameter(param_index).getName();
	return names[param_index].c_str();
}

int parameter_index_from_name (const char *param_name)
{
	const Preset &_preset = _get_preset();
	for (unsigned i=0; i<_preset.ParameterCount(); i++) {
		if (std::string(param_name) == _preset.getParameter(i).getName()) {
			return i;
		}
	}
	return -1;
}

int parameter_get_display (int parameter_index, float parameter_value, char *buffer, size_t maxlen)
{
	const Preset &_preset = _get_preset();
	Parameter parameter = _preset.getParameter(parameter_index);
	parameter.setValue(parameter_value);
	float real_value = parameter.getControlValue();
	
	switch (parameter_index) {
		case kAmsynthParameter_AmpEnvAttack:
		case kAmsynthParameter_AmpEnvDecay:
		case kAmsynthParameter_AmpEnvRelease:
		case kAmsynthParameter_FilterEnvAttack:
		case kAmsynthParameter_FilterEnvDecay:
		case kAmsynthParameter_FilterEnvRelease:
		case kAmsynthParameter_PortamentoTime:
			if (real_value < 1.0) {
				return snprintf(buffer, maxlen, "%.0f ms", real_value * 1000);
			} else {
				return snprintf(buffer, maxlen, "%.1f s", real_value);
			}
		case kAmsynthParameter_LFOFreq:
			return snprintf(buffer, maxlen, "%.1f Hz", real_value);
		case kAmsynthParameter_Oscillator2Detune:
			return snprintf(buffer, maxlen, "%+.1f Cents", 1200.0 * log2(real_value));
		case kAmsynthParameter_Oscillator2Pitch:
			return snprintf(buffer, maxlen, "%+.0f Semitone%s", real_value, fabsf(real_value) < 2 ? "" : "s");
		case kAmsynthParameter_Oscillator2Octave:
			return snprintf(buffer, maxlen, "%+.0f Octave%s", parameter_value, fabsf(parameter_value) < 2 ? "" : "s");
		case kAmsynthParameter_MasterVolume:
			return snprintf(buffer, maxlen, "%+.1f dB", 20.0 * log10(real_value));
		case kAmsynthParameter_OscillatorMixRingMod:
			return snprintf(buffer, maxlen, "%d %%", (int)roundf(real_value * 100.f));
		case kAmsynthParameter_FilterEnvAmount:
			return snprintf(buffer, maxlen, "%+d %%", (int)roundf(real_value / 16.f * 100.f));
		case kAmsynthParameter_AmpEnvSustain:
		case kAmsynthParameter_FilterResonance:
		case kAmsynthParameter_FilterCutoff:
		case kAmsynthParameter_FilterEnvSustain:
		case kAmsynthParameter_LFOToOscillators:
		case kAmsynthParameter_LFOToFilterCutoff:
		case kAmsynthParameter_LFOToAmp:
		case kAmsynthParameter_ReverbRoomsize:
		case kAmsynthParameter_ReverbDamp:
		case kAmsynthParameter_ReverbWet:
		case kAmsynthParameter_ReverbWidth:
		case kAmsynthParameter_AmpDistortion:
		case kAmsynthParameter_FilterKeyTrackAmount:
		case kAmsynthParameter_FilterKeyVelocityAmount:
		case kAmsynthParameter_AmpVelocityAmount:
			return snprintf(buffer, maxlen, "%d %%", (int)roundf(parameter.getNormalisedValue() * 100.f));
		case kAmsynthParameter_FilterType: {
            const char **filter_type_names = parameter_get_value_strings(parameter_index);
            if (filter_type_names) {
                return snprintf(buffer, maxlen, "%s", filter_type_names[(int)real_value]);
            } else {
                strcpy(buffer, "");
                return 0;
            }
        }
	}
	return 0;
}

const char **parameter_get_value_strings (int parameter_index)
{
    static std::vector<std::vector<const char *> > parameterStrings(kAmsynthParameterCount);
    if (parameter_index < 0 || parameter_index >= (int)parameterStrings.size())
        return nullptr;

    std::vector<const char *> & strings = parameterStrings[parameter_index];
    if (strings.empty()) {
        size_t i = 0, size = 0;
        switch (parameter_index) {
            case kAmsynthParameter_Oscillator1Waveform:
            case kAmsynthParameter_Oscillator2Waveform:
                strings.resize(size = 6);
                strings[i++] = _("sine");
                strings[i++] = _("square / pulse");
                strings[i++] = _("triangle / saw");
                strings[i++] = _("white noise");
                strings[i++] = _("noise + sample & hold");
                assert(i < size);
                break;

            case kAmsynthParameter_LFOWaveform:
                strings.resize(size = 8);
                strings[i++] = _("sine");
                strings[i++] = _("square");
                strings[i++] = _("triangle");
                strings[i++] = _("noise");
                strings[i++] = _("noise + sample & hold");
                strings[i++] = _("sawtooth (up)");
                strings[i++] = _("sawtooth (down)");
                assert(i < size);
                break;

            case kAmsynthParameter_KeyboardMode:
                strings.resize(size = 4);
                strings[i++] = _("poly");
                strings[i++] = _("mono");
                strings[i++] = _("legato");
                assert(i < size);
                break;

            case kAmsynthParameter_FilterType:
                strings.resize(size = 6);
                strings[i++] = _("low pass");
                strings[i++] = _("high pass");
                strings[i++] = _("band pass");
                strings[i++] = _("notch");
                strings[i++] = _("bypass");
                assert(i < size);
                break;

            case kAmsynthParameter_FilterSlope:
                strings.resize(size = 3);
                strings[i++] = _("12 dB / octave");
                strings[i++] = _("24 dB / octave");
                assert(i < size);
                break;

            case kAmsynthParameter_LFOOscillatorSelect:
                strings.resize(size = 4);
                strings[i++] = _("osc 1+2");
                strings[i++] = _("osc 1");
                strings[i++] = _("osc 2");
                assert(i < size);
                break;

            case kAmsynthParameter_PortamentoMode:
                strings.resize(size = 3);
                strings[i++] = _("always");
                strings[i++] = _("legato");
                assert(i < size);
                break;

            default:
                break;
        }
    }

    return &strings[0];
}

static std::vector<bool> s_ignoreParameter(kAmsynthParameterCount);

bool Preset::shouldIgnoreParameter(int parameter)
{
	assert(parameter >= 0 && parameter < (int)s_ignoreParameter.size());
	return s_ignoreParameter[parameter];
}

void Preset::setShouldIgnoreParameter(int parameter, bool ignore)
{
	assert(parameter >= 0 && parameter < (int)s_ignoreParameter.size());
	s_ignoreParameter[parameter] = ignore;
}

std::string Preset::getIgnoredParameterNames()
{
	const Preset &_preset = _get_preset();
	std::string names;
	for (int i = 0; i < kAmsynthParameterCount; i++) {
		if (shouldIgnoreParameter(i)) {
			if (!names.empty())
				names += " ";
			names += _preset.getParameter(i).getName();
		}
	}
	return names;
}

void Preset::setIgnoredParameterNames(std::string names)
{
	for (int i = 0; i < kAmsynthParameterCount; i++) {
		setShouldIgnoreParameter(i, false);
	}

	std::stringstream ss(names);
	std::istream_iterator<std::string> begin(ss);
	std::istream_iterator<std::string> end;
	std::vector<std::string> vstrings(begin, end);

	std::vector<std::string>::const_iterator name_it;
	for (name_it = vstrings.begin(); name_it != vstrings.end(); ++name_it) {
		int index = parameter_index_from_name(name_it->c_str());
		if (index != -1) {
			setShouldIgnoreParameter(index, true);
		}
	}
}
