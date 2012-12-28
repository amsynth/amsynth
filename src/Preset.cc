/*
 *  Preset.cc
 *
 *  Copyright (c) 2001-2012 Nick Dowell
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

#include "controls.h"
#include "VoiceBoard/LowPassFilter.h"

#include <cstdlib>
#include <cstdio>

#ifdef _DEBUG
#include <iostream>
using namespace std;
#endif

Parameter TimeParameter (const string name, Param id)
{
	return Parameter (name, id, 0, 0, 2.5f, 0, Parameter::PARAM_POWER, 3, 0.0005f, "s");
}

const char *osc_waveform_names[] = {
	"sine", "square / pulse", "saw / triangle", "white noise", "noise + sample & hold", NULL
};

const char *lfo_waveform_names[] = {
	"sine", "square", "triangle", "noise", "noise + sample & hold", "sawtooth (up)", "sawtooth (down)", NULL
};

const char *keyboard_mode_names[] = {
	"poly", "mono", "legato", NULL
};

const char *filter_type_names[] = {
	"low pass", "high pass", "band pass", NULL
};

const char *filter_slope_names[] = {
	"12 dB / octave", "24 dB / octave", NULL
};

Preset::Preset			(const string name)
:	mName (name)
,	nullparam ("null", kAmsynthParameterCount)
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
	mParameters.push_back (Parameter		("filter_cutoff",		kAmsynthParameter_FilterCutoff,		1.5, -0.5, 1.5, 0,	Parameter::PARAM_EXP, 16, 0));
    mParameters.push_back (Parameter		("osc2_detune",			kAmsynthParameter_Oscillator2Detune,		0, -1, 1, 0,		Parameter::PARAM_EXP, 1.25f, 0));
    mParameters.push_back (Parameter		("osc2_waveform",		kAmsynthParameter_Oscillator2Waveform,		2, 0, 4, 1));
    mParameters.push_back (Parameter		("master_vol",			kAmsynthParameter_MasterVolume,			0.67, 0, 1, 0,		Parameter::PARAM_POWER, 2, 0));
    mParameters.push_back (Parameter		("lfo_freq",			kAmsynthParameter_LFOFreq,			0, 0, 7.5, 0,		Parameter::PARAM_POWER, 2, 0,	"Hz"));
    mParameters.push_back (Parameter		("lfo_waveform",		kAmsynthParameter_LFOWaveform,		0, 0, 6, 1));
    mParameters.push_back (Parameter		("osc2_range",			kAmsynthParameter_Oscillator2Octave,		0, -1, 2, 1,		Parameter::PARAM_EXP, 2, 0));
	mParameters.push_back (Parameter		("osc_mix",				kAmsynthParameter_OscillatorMix,			0, -1, 1));
	mParameters.push_back (Parameter		("freq_mod_amount",		kAmsynthParameter_LFOToOscillators,		0, 0, 1.25992105f,0,Parameter::PARAM_POWER, 3, -1));
	mParameters.push_back (Parameter		("filter_mod_amount",	kAmsynthParameter_LFOToFilterCutoff,	-1, -1, 1));
	mParameters.push_back (Parameter		("amp_mod_amount",		kAmsynthParameter_LFOToAmp,		-1, -1, 1));
	mParameters.push_back (Parameter		("osc_mix_mode",		kAmsynthParameter_OscillatorMixRingMod,		0, 0, 1, 1));
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
	mParameters.push_back (Parameter		("filter_type",         kAmsynthParameter_FilterType, SynthFilter::FilterTypeLowPass, SynthFilter::FilterTypeLowPass, SynthFilter::FilterTypeCount - 1, 1));
	mParameters.push_back (Parameter		("filter_slope",        kAmsynthParameter_FilterSlope, SynthFilter::FilterSlope24, SynthFilter::FilterSlope12, SynthFilter::FilterSlope24, 1));

	getParameter(kAmsynthParameter_Oscillator1Waveform).setValueStrings(osc_waveform_names);
	getParameter(kAmsynthParameter_Oscillator2Waveform).setValueStrings(osc_waveform_names);
	getParameter(kAmsynthParameter_LFOWaveform).setValueStrings(lfo_waveform_names);
	getParameter(kAmsynthParameter_KeyboardMode).setValueStrings(keyboard_mode_names);
	getParameter(kAmsynthParameter_FilterType).setValueStrings(filter_type_names);
	getParameter(kAmsynthParameter_FilterSlope).setValueStrings(filter_slope_names);
}

Preset&
Preset::operator =		(Preset& preset)
{
    for (unsigned i=0; i<preset.ParameterCount(); i++)
    {
		Parameter& p = preset.getParameter (i);
		getParameter (p.getName()).setValue (p.getValue());
    }
    setName (preset.getName());
    return *this;
}

bool
Preset::isEqual(Preset &otherPreset)
{
	for (unsigned i = 0; i < mParameters.size(); i++) {
		if (            getParameter(i).getValue() !=
			otherPreset.getParameter(i).getValue()) {
			return false;
		}
	}
	return getName() == otherPreset.getName();
}

Parameter & 
Preset::getParameter(const string name)
{
    for (unsigned i = 0; i < mParameters.size(); i++) if (getParameter(i).getName() == name) return mParameters[i];
    return nullparam;
}

void
Preset::randomise()
{
	float master_vol = getParameter("master_vol").getValue ();
	for (unsigned i=0; i<mParameters.size(); i++) getParameter(i).random_val();
	getParameter("master_vol").setValue (master_vol);
}

void
Preset::AddListenerToAll	(UpdateListener* ul)
{
	for (unsigned i=0; i<mParameters.size(); i++) getParameter(i).addUpdateListener (*ul);
}

string
Preset::toString()
{
	std::stringstream stream;
	stream << "amSynth1.0preset" << std::endl;
	stream << "<preset> " << "<name> " << getName() << std::endl;
	for (unsigned n = 0; n < ParameterCount(); n++) {
		stream << "<parameter> " << getParameter(n).getName() << " " << getParameter(n).getValue() << std::endl;
	}
	return stream.str();
}

bool
Preset::fromString(string str)
{
	std::stringstream stream (str);

	string buffer;
  
	stream >> buffer;
  
	if (buffer != "amSynth1.0preset") return false;
  
	stream >> buffer;
	if (buffer == "<preset>") {
		stream >> buffer;
		
		//get the preset's name
		stream >> buffer;
		string presetName;
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
			string name;
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

static Preset _preset;

const char *parameter_name_from_index (int param_index)
{
	if (param_index >= (int)_preset.ParameterCount())
		return NULL;
	return _preset.getParameter(param_index).getName().c_str();
}

int parameter_index_from_name (const char *param_name)
{
	for (unsigned i=0; i<_preset.ParameterCount(); i++) {
		if (std::string(param_name) == _preset.getParameter(i).getName()) {
			return i;
		}
	}
	return -1;
}

int parameter_get_display (int parameter_index, float parameter_value, char *buffer, size_t maxlen)
{
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
			return snprintf(buffer, maxlen, "%+.0f Semitones", real_value);
		case kAmsynthParameter_AmpEnvSustain:
		case kAmsynthParameter_MasterVolume:
			return snprintf(buffer, maxlen, "%+.1f dB", 20.0 * log10(real_value));
		case kAmsynthParameter_FilterEnvAmount:
			return snprintf(buffer, maxlen, "%+d %%", (int)roundf(real_value / 16.0 * 100.0));
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
			return snprintf(buffer, maxlen, "%d %%", (int)roundf(parameter.GetNormalisedValue() * 100.0));
			break;
		case kAmsynthParameter_FilterType:
			return snprintf(buffer, maxlen, "%s", filter_type_names[(int)real_value]);
	}
	return 0;
}

const char **parameter_get_value_strings (int parameter_index)
{
	Parameter parameter = _preset.getParameter(parameter_index);
	const char **value_strings = parameter.valueStrings();
	return value_strings;
}
