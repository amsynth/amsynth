/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#include "Preset.h"
#include <cstdlib>

#ifdef _DEBUG
#include <iostream>
using namespace std;
#endif

Parameter TimeParameter (const string name, Param id)
{
	return Parameter (name, id, 0, 0, 2.5f, 0, Parameter::PARAM_POWER, 3, 0.0005f, "s");
}

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
    mParameters.push_back (Parameter		("lfo_waveform",		kAmsynthParameter_LFOWaveform,		0, 0, 4, 1));
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

