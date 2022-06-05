/*
 *  Parameter.h
 *
 *  Copyright (c) 2001-2022 Nick Dowell
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

#include "Parameter.h"

#include "VoiceBoard/Synth--.h"

#include <cassert>
#include <sstream>

#if defined _MSC_VER // does not support Designated Initializers
#define SPEC(id, name, def, min, max, step, law, base, offset, label)        { name, def, min, max,  step, law, base, offset, label }
#else
#define SPEC(id, name, def, min, max, step, law, base, offset, label) [id] = { name, def, min, max,  step, law, base, offset, label }
#endif

static const ParameterSpec ParameterSpecs[] = { //                            def,    min,    max,  step,       law,                         base,  offset,     label
	SPEC(kAmsynthParameter_AmpEnvAttack,            "amp_attack",            0.0f,   0.0f,   2.5f,  0.0f,       kParameterLaw_Power,         3.0f,  0.0005f,    "s"  ),
	SPEC(kAmsynthParameter_AmpEnvDecay,             "amp_decay",             0.0f,   0.0f,   2.5f,  0.0f,       kParameterLaw_Power,         3.0f,  0.0005f,    "s"  ),
	SPEC(kAmsynthParameter_AmpEnvSustain,           "amp_sustain",           1.0f,   0.0f,   1.0f,  0.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_AmpEnvRelease,           "amp_release",           0.0f,   0.0f,   2.5f,  0.0f,       kParameterLaw_Power,         3.0f,  0.0005f,    "s"  ),
	SPEC(kAmsynthParameter_Oscillator1Waveform,     "osc1_waveform",         2.0f,   0.0f,   4.0f,  1.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_FilterEnvAttack,         "filter_attack",         0.0f,   0.0f,   2.5f,  0.0f,       kParameterLaw_Power,         3.0f,  0.0005f,    "s"  ),
	SPEC(kAmsynthParameter_FilterEnvDecay,          "filter_decay",          0.0f,   0.0f,   2.5f,  0.0f,       kParameterLaw_Power,         3.0f,  0.0005f,    "s"  ),
	SPEC(kAmsynthParameter_FilterEnvSustain,        "filter_sustain",        1.0f,   0.0f,   1.0f,  0.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_FilterEnvRelease,        "filter_release",        0.0f,   0.0f,   2.5f,  0.0f,       kParameterLaw_Power,         3.0f,  0.0005f,    "s"  ),
	SPEC(kAmsynthParameter_FilterResonance,         "filter_resonance",      0.0f,   0.0f,   0.97f, 0.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_FilterEnvAmount,         "filter_env_amount",     0.0f, -16.0f,  16.0f,  0.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_FilterCutoff,            "filter_cutoff",         1.5f,  -0.5f,   1.5f,  0.0f,       kParameterLaw_Exponential,  16.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_Oscillator2Detune,       "osc2_detune",           0.0f,  -1.0f,   1.0f,  0.0f,       kParameterLaw_Exponential,   1.25f, 0.0f,       ""   ),
	SPEC(kAmsynthParameter_Oscillator2Waveform,     "osc2_waveform",         2.0f,   0.0f,   4.0f,  1.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_MasterVolume,            "master_vol",            0.67f,  0.0f,   1.0f,  0.0f,       kParameterLaw_Power,         2.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_LFOFreq,                 "lfo_freq",              0.0f,   0.0f,   7.5f,  0.0f,       kParameterLaw_Power,         2.0f,  0.0f,       "Hz" ),
	SPEC(kAmsynthParameter_LFOWaveform,             "lfo_waveform",          0.0f,   0.0f,   6.0f,  1.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_Oscillator2Octave,       "osc2_range",            0.0f,  -3.0f,   4.0f,  1.0f,       kParameterLaw_Exponential,   2.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_OscillatorMix,           "osc_mix",               0.0f,  -1.0f,   1.0f,  0.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_LFOToOscillators,        "freq_mod_amount",       0.0f,   0.0f, 1.25992105f, 0.0f,   kParameterLaw_Power,         3.0f, -1.0f,       ""   ),
	SPEC(kAmsynthParameter_LFOToFilterCutoff,       "filter_mod_amount",    -1.0f,  -1.0f,   1.0f,  0.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_LFOToAmp,                "amp_mod_amount",       -1.0f,  -1.0f,   1.0f,  0.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_OscillatorMixRingMod,    "osc_mix_mode",          0.0f,   0.0f,   1.0f,  0.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_Oscillator1Pulsewidth,   "osc1_pulsewidth",       1.0f,   0.0f,   1.0f,  0.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_Oscillator2Pulsewidth,   "osc2_pulsewidth",       1.0f,   0.0f,   1.0f,  0.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_ReverbRoomsize,          "reverb_roomsize",       0.0f,   0.0f,   1.0f,  0.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_ReverbDamp,              "reverb_damp",           0.0f,   0.0f,   1.0f,  0.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_ReverbWet,               "reverb_wet",            0.0f,   0.0f,   1.0f,  0.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_ReverbWidth,             "reverb_width",          1.0f,   0.0f,   1.0f,  0.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_AmpDistortion,           "distortion_crunch",     0.0f,   0.0f,   0.9f,  0.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_Oscillator2Sync,         "osc2_sync",             0.0f,   0.0f,   1.0f,  1.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_PortamentoTime,          "portamento_time",       0.0f,   0.0f,   1.0f,  0.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_KeyboardMode,            "keyboard_mode",         0.0f,   0.0f,   2.0f,  1.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_Oscillator2Pitch,        "osc2_pitch",            0.0f, -12.0f,  12.0f,  1.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_FilterType,              "filter_type",           0.0f,   0.0f,   4.0f,  1.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_FilterSlope,             "filter_slope",          1.0f,   0.0f,   1.0f,  1.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_LFOOscillatorSelect,     "freq_mod_osc",          0.0f,   0.0f,   2.0f,  1.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_FilterKeyTrackAmount,    "filter_kbd_track",      1.0f,   0.0f,   1.0f,  0.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_FilterKeyVelocityAmount, "filter_vel_sens",       1.0f,   0.0f,   1.0f,  0.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_AmpVelocityAmount,       "amp_vel_sens",          1.0f,   0.0f,   1.0f,  0.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
	SPEC(kAmsynthParameter_PortamentoMode,          "portamento_mode",       0.0f,   0.0f,   1.0f,  0.0f,       kParameterLaw_Linear,        1.0f,  0.0f,       ""   ),
};

Parameter::Parameter(Param paramId)
:	_paramId	(paramId)
,	_spec		(ParameterSpecs[paramId])
,	_value		(_spec.def)
{}

void
Parameter::addUpdateListener(UpdateListener *listener)
{
	_listeners.insert(listener);
	listener->UpdateParameter(_paramId, getControlValue());
}

void
Parameter::removeUpdateListener(UpdateListener *listener)
{
	_listeners.erase(listener);
}

void
Parameter::setValue(float value)
{
	if (value == _value) return;
	
	float newValue = std::min(std::max(value, _spec.min), _spec.max);

	if (_spec.step > 0.f) {
		newValue = _spec.min + roundf((newValue - _spec.min) / _spec.step) * _spec.step;
		assert(::fmodf(newValue - _spec.min, _spec.step) == 0);
	}

	if (_value == newValue) // warning: -ffast-math causes this comparison to fail
		return;

	_value = newValue;

	float cv = getControlValue();
	for (auto &listener : _listeners) {
		listener->UpdateParameter(_paramId, cv);
	}
}

float
Parameter::getControlValue() const
{
	switch (_spec.law) {
		case kParameterLaw_Linear:
			return _spec.offset + _spec.base * _value;
		case kParameterLaw_Exponential:
			return _spec.offset + ::pow((float)_spec.base, _value);
		case kParameterLaw_Power:
			return _spec.offset + ::pow(_value, (float)_spec.base);
		default:
			abort();
	}
}

float
Parameter::valueFromString(const std::string &str)
{
	// atof() and friends are affected by currently configured locale,
	// which can change the decimal point character.
	std::istringstream istr(str);
	static std::locale locale = std::locale("C");
	istr.imbue(locale);
	float value = m::nan;
	istr >> value;
	return value;
}

const std::string
Parameter::getStringValue() const
{
	std::ostringstream stream;
	stream << getControlValue();
	return stream.str();
}

void
Parameter::randomise()
{
	setValue( ((rand()/(float)RAND_MAX) * (getMax()-getMin()) + getMin()) );
}
