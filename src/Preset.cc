/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#include "Preset.h"

void setupTimeParam (Parameter& p) { p.setType( PARAM_POWER, 3, 0.0005 ); p.setMin( 0.0 );	p.setMax( 2.5 ); p.setLabel("s"); }

Preset::Preset()
:	mName ("New Preset")
,	nullparam ("null", kControls_End)
{
	mParameters.push_back (Parameter ("amp_attack", kAmpAttack));
	setupTimeParam (mParameters.back());
	
	mParameters.push_back (Parameter ("amp_decay", kAmpDecay));
    setupTimeParam (mParameters.back());

    mParameters.push_back (Parameter ("amp_sustain", kAmpSustain));
    mParameters.back().setValue(1);

    mParameters.push_back (Parameter ("amp_release", kAmpRelease));
    setupTimeParam (mParameters.back());

    mParameters.push_back (Parameter ("osc1_waveform", kOsc1Waveform));
    mParameters.back().setMin(0.0);
    mParameters.back().setMax(4.0);
    mParameters.back().setStep(1.0);

    mParameters.push_back (Parameter ("filter_attack", kFilterAttack));
    setupTimeParam (mParameters.back());

    mParameters.push_back (Parameter ("filter_decay", kFilterDecay));
    setupTimeParam (mParameters.back());

    mParameters.push_back (Parameter ("filter_sustain", kFilterSustain));
    mParameters.back().setValue(1);

    mParameters.push_back (Parameter ("filter_release", kFilterRelease));
    setupTimeParam (mParameters.back());

    mParameters.push_back (Parameter ("filter_resonance", kFilterResonance));
	mParameters.back().setMax(0.97);
    mParameters.back().setValue(0);
	
	mParameters.push_back (Parameter ("filter_env_amount", kFilterEnvAmount));
	mParameters.back().setMax(16.0);
    mParameters.back().setValue(0);
    
	mParameters.push_back (Parameter ("filter_cutoff", kFilterCutoff));
    mParameters.back().setType(PARAM_EXP, 16, 0);
    mParameters.back().setMin(-0.5);
    mParameters.back().setMax(1);
    mParameters.back().setValue(1);

    mParameters.push_back (Parameter ("osc2_detune", kOsc2Detune));
    mParameters.back().setType(PARAM_EXP, 1.25, 0);
    mParameters.back().setMin(-1);
    mParameters.back().setMax(1);
    mParameters.back().setValue(0);

    mParameters.push_back (Parameter ("osc2_waveform", kOsc2Waveform));
    mParameters.back().setMin(0.0);
    mParameters.back().setMax(4.0);
    mParameters.back().setStep(1.0);

    mParameters.push_back (Parameter ("master_vol", kMasterVol));
    mParameters.back().setMin(0.0);
    mParameters.back().setMax(1.0);
    mParameters.back().setValue(1.0);
	mParameters.back().setType( PARAM_POWER, 2, 0 );

    mParameters.push_back (Parameter ( "lfo_freq", kLFOFreq ));
	mParameters.back().setType( PARAM_POWER, 2, 0 );
    mParameters.back().setMax( 7.5 );
	mParameters.back().setLabel( "Hz" );

    mParameters.push_back (Parameter ( "lfo_depth", kLFODepth ));
    mParameters.back().setValue( 0 );

    mParameters.push_back (Parameter ("lfo_waveform", kLFOWaveform));
    mParameters.back().setMin(0.0);
    mParameters.back().setMax(4.0);
    mParameters.back().setStep(1.0);

    mParameters.push_back (Parameter ("osc2_range", kOsc2Octave));
    mParameters.back().setMin(-1);
    mParameters.back().setMax(2);
    mParameters.back().setStep(1.0);
    mParameters.back().setType(PARAM_EXP, 2, 0);
    mParameters.back().setValue(0);
	
	mParameters.push_back (Parameter ("osc_mix", kOscMix));
    mParameters.back().setMin(-1.0);
    mParameters.back().setMax(1.0);
	
	mParameters.push_back (Parameter ("freq_mod_amount", kFreqModAmount));
	mParameters.back().setType( PARAM_POWER, 3, -1 );
	mParameters.back().setMin( 0.0 );
	mParameters.back().setMax( 1.25992105 );
	
	mParameters.push_back (Parameter ("filter_mod_amount", kFilterModAmount));
    mParameters.back().setMin(-1.0);
    mParameters.back().setMax(1.0);
	mParameters.back().setValue(-1);

	mParameters.push_back (Parameter ("amp_mod_amount", kAmpModAmount));
    mParameters.back().setMin(-1.0);
    mParameters.back().setMax(1.0);
	mParameters.back().setValue(-1);
	
	mParameters.push_back (Parameter ("osc1_pwm_amount", kOsc1PWMAmount));
    mParameters.back().setMin(-1.0);
    mParameters.back().setMax(1.0);
	mParameters.back().setValue(-1);
	
	mParameters.push_back (Parameter ("osc_mix_mode", kOscMixRingMod));
    mParameters.back().setMin(0.0);
    mParameters.back().setMax(1.0);
	mParameters.back().setStep(1.0);
	
	mParameters.push_back (Parameter ("osc1_pulsewidth", kOsc1Pulsewidth));
    mParameters.back().setMin(0.0);
    mParameters.back().setMax(1.0);
	
	mParameters.push_back (Parameter ("osc2_pulsewidth", kOsc2Pulsewidth));
    mParameters.back().setMin(0.0);
    mParameters.back().setMax(1.0);
	
	mParameters.push_back (Parameter ("reverb_roomsize", kReverbRoomsize));
    mParameters.back().setMin(0.0);
    mParameters.back().setMax(1.0);
	
	mParameters.push_back (Parameter ("reverb_damp", kReverbDamp));
    mParameters.back().setMin(0.0);
    mParameters.back().setMax(1.0);
	
	mParameters.push_back (Parameter ("reverb_wet", kReverbWet));
    mParameters.back().setMin( 0.0 );
    mParameters.back().setMax( 1.0 );
	mParameters.back().setValue( 0.0 );
	
	mParameters.push_back (Parameter ("reverb_dry", kReverbDry));
    mParameters.back().setMin(0.0);
    mParameters.back().setMax(1.0);
	mParameters.back().setValue(0.5);
	
	mParameters.push_back (Parameter ("reverb_width", kReverbWidth));
    mParameters.back().setMin(0.0);
    mParameters.back().setMax(1.0);
	
	mParameters.push_back (Parameter ("reverb_mode", kReverbMode));
    mParameters.back().setMin(0.0);
    mParameters.back().setMax(0.0);
	mParameters.back().setDiscrete(true);
	
	mParameters.push_back (Parameter ("distortion_drive", kDistortionDrive));
    mParameters.back().setMin(1.0);
    mParameters.back().setMax(16.0);
	mParameters.back().setValue(1.0);
	
	mParameters.push_back (Parameter ("distortion_crunch", kDistortionCrunch));
	mParameters.back().setMin(0.0);
	mParameters.back().setMax(0.9);
	mParameters.back().setValue (0.0);
	
	mParameters.push_back (Parameter ("osc2_sync", kOsc2Sync));
    mParameters.back().setMin(0.0);
    mParameters.back().setMax(1.0);
	mParameters.back().setStep(1.0);
}

void
Preset::clone(Preset & preset)
{
    for (unsigned i = 0; i < mParameters.size(); i++) {
		mParameters[i].setValue(preset.getParameter(i).getValue());
		mParameters[i].setName(
			preset.getParameter(i).getName (),
			preset.getParameter(i).GetId ());
    }
    setName(preset.getName());
}

Parameter & 
Preset::getParameter(string name)
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
