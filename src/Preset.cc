/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#include "Preset.h"

void setupTimeParam (Parameter& p) { p.setType( Parameter::PARAM_POWER, 3, 0.0005 ); p.setMin( 0.0 );	p.setMax( 2.5 ); p.setLabel("s"); }
Parameter TimeParameter (string name, Param id)
{
	Parameter p (name, id);
	p.setType( Parameter::PARAM_POWER, 3, 0.0005 );
	p.setMin( 0.0 );
	p.setMax( 2.5 );
	p.setLabel("s");
	return p;
}

Preset::Preset()
:	mName ("New Preset")
,	nullparam ("null", kControls_End)
{
	mParameters.push_back (TimeParameter	("amp_attack", kAmpAttack));
	mParameters.push_back (TimeParameter	("amp_decay", kAmpDecay));
    mParameters.push_back (Parameter		("amp_sustain", kAmpSustain, 1));
    mParameters.push_back (TimeParameter	("amp_release", kAmpRelease));
    mParameters.push_back (Parameter		("osc1_waveform", kOsc1Waveform, 0, 0, 4));
    mParameters.back().setStep(1.0);
    mParameters.push_back (TimeParameter	("filter_attack", kFilterAttack));
    mParameters.push_back (TimeParameter	("filter_decay", kFilterDecay));
    mParameters.push_back (Parameter		("filter_sustain", kFilterSustain, 1));
    mParameters.push_back (TimeParameter	("filter_release", kFilterRelease));
    mParameters.push_back (Parameter		("filter_resonance", kFilterResonance, 0, 0, 0.97));
	mParameters.push_back (Parameter		("filter_env_amount", kFilterEnvAmount, 0, 0, 16));
	mParameters.push_back (Parameter		("filter_cutoff", kFilterCutoff));
    mParameters.back().setType(Parameter::PARAM_EXP, 16, 0);
    mParameters.back().setMin(-0.5);
    mParameters.back().setMax(1);
    mParameters.back().setValue(1);
    mParameters.push_back (Parameter ("osc2_detune", kOsc2Detune));
    mParameters.back().setType(Parameter::PARAM_EXP, 1.25, 0);
    mParameters.back().setMin(-1);
    mParameters.back().setMax(1);
    mParameters.back().setValue(0);
    mParameters.push_back (Parameter		("osc2_waveform", kOsc2Waveform, 0, 0, 4));
    mParameters.back().setStep(1.0);
    mParameters.push_back (Parameter		("master_vol", kMasterVol, 1, 0, 1));
	mParameters.back().setType( Parameter::PARAM_POWER, 2, 0 );
    mParameters.push_back (Parameter		("lfo_freq", kLFOFreq, 0, 0, 7.5));
	mParameters.back().setType( Parameter::PARAM_POWER, 2, 0 );
	mParameters.back().setLabel( "Hz" );
    mParameters.push_back (Parameter		("lfo_depth", kLFODepth, 0));
    mParameters.push_back (Parameter		("lfo_waveform", kLFOWaveform, 0, 0, 4));
    mParameters.back().setStep(1.0);
    mParameters.push_back (Parameter		("osc2_range", kOsc2Octave, 0, -1, 2));
    mParameters.back().setStep(1.0);
    mParameters.back().setType(Parameter::PARAM_EXP, 2, 0);
	mParameters.push_back (Parameter		("osc_mix", kOscMix, 0, -1, 1));
	mParameters.push_back (Parameter		("freq_mod_amount", kFreqModAmount, 0, 0, 1.25992105));
	mParameters.back().setType( Parameter::PARAM_POWER, 3, -1 );
	mParameters.push_back (Parameter		("filter_mod_amount", kFilterModAmount, -1, -1, 1));
	mParameters.push_back (Parameter		("amp_mod_amount", kAmpModAmount, -1, -1, 1));
	mParameters.push_back (Parameter		("osc1_pwm_amount", kOsc1PWMAmount, -1, -1, 1));
	mParameters.push_back (Parameter		("osc_mix_mode", kOscMixRingMod));
	mParameters.back().setStep(1.0);
	mParameters.push_back (Parameter		("osc1_pulsewidth", kOsc1Pulsewidth));
	mParameters.push_back (Parameter		("osc2_pulsewidth", kOsc2Pulsewidth));
	mParameters.push_back (Parameter		("reverb_roomsize", kReverbRoomsize));
	mParameters.push_back (Parameter		("reverb_damp", kReverbDamp));
	mParameters.push_back (Parameter		("reverb_wet", kReverbWet));
	mParameters.push_back (Parameter		("reverb_dry", kReverbDry, 0.5));
	mParameters.push_back (Parameter		("reverb_width", kReverbWidth));
	mParameters.push_back (Parameter		("reverb_mode", kReverbMode));
	mParameters.back().setDiscrete(true);
	mParameters.push_back (Parameter		("distortion_drive", kDistortionDrive, 1, 1, 16));
	mParameters.push_back (Parameter		("distortion_crunch", kDistortionCrunch, 0, 0, 0.9));
	mParameters.push_back (Parameter		("osc2_sync", kOsc2Sync));
	mParameters.back().setStep(1.0);
}

void
Preset::clone		(Preset& preset)
{
    for (unsigned i=0; i<preset.ParameterCount(); i++)
    {
    	Parameter& p = preset.getParameter(i);
    	getParameter(p.getName()).setValue (p.getValue());
    }
    setName(preset.getName());
}

Parameter & 
Preset::getParameter(const string name)
{
    for (unsigned i = 0; i < mParameters.size(); i++) {
		if (mParameters[i].getName() == name) {
#ifdef _DEBUG
			cout << "<Preset::getParameter()> Parameter '" << name
			<< "' found" << endl;
#endif
			return mParameters[i];
		}
    }
#ifdef _DEBUG
    cout << "<Preset::getParameter()> Parameter '" << name
	<< "' not found" << endl;
#endif
    return nullparam;
}

void
Preset::randomise()
{
	float master_vol = getParameter( "master_vol" ).getValue();
	for (unsigned i=0; i<mParameters.size(); i++) 
		mParameters[i].random_val();
	getParameter( "master_vol" ).setValue( master_vol );
}

void
Preset::AddListenerToAll	(UpdateListener* ul)
{
	for (unsigned i=0; i<mParameters.size(); i++) mParameters[i].addUpdateListener (*ul);
}
