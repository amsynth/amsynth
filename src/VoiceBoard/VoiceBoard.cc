/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "VoiceBoard.h"

VoiceBoard::VoiceBoard(int rate):
	// call object constructors with parameters
	mod_lfo(rate),
	osc1(rate), 
	osc2(rate), 
	filter(rate), 
	filter_env(rate), 
	amp_env(rate)
{
    this->rate = rate;

    half.setValue(0.5);
    one.setValue(1.0);
    two.setValue(2.0);
}

VoiceBoard::~VoiceBoard()
{
}

void
VoiceBoard::init()
{
    /*
     * modulation section
     */
    lfo_freq.setParameter(_preset->getParameter("lfo_freq"));
    mod_lfo.setInput(lfo_freq);
    mod_lfo.setWaveform(_preset->getParameter("lfo_waveform"));

    mod_mult.addInput(mod_lfo);
    mod_mult.addInput(half);
    mod_add.addInput(mod_mult);
    mod_add.addInput(half);

    mod_signal.setInput(mod_add);

    /*
     * pitch control section
     */
    freq_mod_mult.addInput(mod_signal);
    freq_mod_mult.addInput(two);

    freq_mod_mix.setInput1(one);
    freq_mod_mix.setInput2(freq_mod_mult);
    freq_mod_mix_amount.setParameter(_preset->
				     getParameter("freq_mod_amount"));
    freq_mod_mix.setControl(freq_mod_mix_amount);

    freq.addInput(freq_mod_mix);
    freq.addInput(*pitch_bend);
    freq.addInput(key_pitch);

    master_freq.setInput(freq);

    /* 
     * oscillator section
     */
	osc1_pulsewidth_control.setParameter( _preset->getParameter("osc1_pulsewidth") );
	// PULSEWIDTH MODULATION:
	/*
	osc1_pw.addInput( osc1_pw_mixer );
	osc1_pw_mixer.setInput1( one );
	osc1_pw_mixer.setInput2( mod_signal );
	osc1_pw_mixer.setControl( osc1_pwm_amt );
	osc1_pw.addInput( osc1_pulsewidth_control );
	osc1_pwm_amt.setParameter( _preset->getParameter("osc1_pwm_amount") );
	osc1.setPulseWidth( osc1_pw );
	*/
	osc1.setPulseWidth( osc1_pulsewidth_control );
    osc1.setWaveform(_preset->getParameter("osc1_waveform"));
    osc1.setInput(master_freq);
	
	osc1.setSync( _preset->getParameter("osc2_sync"), osc2 );

    osc2_freq.addInput(master_freq);
    osc2_detune.setParameter(_preset->getParameter("osc2_detune"));
    osc2_freq.addInput(osc2_detune);
    osc2_range.setParameter(_preset->getParameter("osc2_range"));
    osc2_freq.addInput(osc2_range);

    osc2.setWaveform(_preset->getParameter("osc2_waveform"));
	osc2_pulsewidth_control.setParameter( _preset->getParameter("osc2_pulsewidth") );
	osc2.setPulseWidth( osc2_pulsewidth_control );
    osc2.setInput(osc2_freq);

    osc_mix.setParameter(_preset->getParameter("osc_mix"));
    osc_mixer.setControl(osc_mix);
    osc_mixer.setInput1(osc1);
    osc_mixer.setInput2(osc2);
	osc_mixer.setMode( _preset->getParameter("osc_mix_mode") );

    /*
     * filter section
     */
    filter_env.setAttack( _preset->getParameter("filter_attack") );
    filter_env.setDecay( _preset->getParameter("filter_decay") );
    filter_env.setSustain( _preset->getParameter("filter_sustain") );
    filter_env.setRelease( _preset->getParameter("filter_release") );

    cutoff_control.setParameter( _preset->getParameter("filter_cutoff") );

    filter_cutoff_mixer.setInput1(one);
    filter_cutoff_mixer.setInput2(mod_signal);
    filter_mod_amount.setParameter(_preset->
				   getParameter("filter_mod_amount"));
    filter_cutoff_mixer.setControl(filter_mod_amount);

    //filter_cutoff.addInput(filter_env);
    filter_cutoff.addInput(key_pitch);
    filter_cutoff.addInput(cutoff_control);
    filter_cutoff.addInput(filter_cutoff_mixer);
	
	filter_env_signal.addInput( filter_env );
	filter_env_signal.addInput( key_pitch );
	filter_env_signal.addInput( vel );
	filter_env_amount.setParameter( _preset->getParameter("filter_env_amount") );
	filter_env_signal.addInput( filter_env_amount );
	
	filter_cutoff_final.addInput( filter_cutoff );
	filter_cutoff_final.addInput( filter_env_signal );

    filter.setCFreq( filter_cutoff_final );
    filter.setResonance(_preset->getParameter("filter_resonance"));
    filter.setInput(osc_mixer);


    /* 
     * amp section
     */
    amp_env.setAttack( _preset->getParameter("amp_attack") );
    amp_env.setDecay( _preset->getParameter("amp_decay") );
    amp_env.setSustain( _preset->getParameter("amp_sustain") );
    amp_env.setRelease( _preset->getParameter("amp_release") );

    amp_mod_amount.setParameter( _preset->getParameter("amp_mod_amount") );
    amp_mixer.setInput1(one);
    amp_mixer.setInput2(mod_signal);
    amp_mixer.setControl(amp_mod_amount);

    amp.addInput( amp_mixer );
    amp.addInput( amp_env );
	amp.addInput( vel );
    amp.addInput( filter );
}

float *VoiceBoard::getNFData()
{
    mod_signal.process();
    master_freq.process();
    return amp.getFData();
}

void VoiceBoard::setPitchWheel(FSource & source)
{
    pitch_bend = &source;
}

int 
VoiceBoard::getState()
{
	/*
    if( amp_env.getState() == 0 && filter_env.getState() == 0 ){
		//cout << "VoiceBoard " << key_pitch.getValue() << " is off\n";
		return 0;
	}
	return 1;
	*/
	return amp_env.getState();
}

void 
VoiceBoard::triggerOn()
{
    amp_env.triggerOn();
    filter_env.triggerOn();
}

void 
VoiceBoard::triggerOff()
{
    amp_env.triggerOff();
    filter_env.triggerOff();
}

void
VoiceBoard::reset()
{
	amp_env.reset();
	filter_env.reset();
	osc1.reset();
	osc2.reset();
	filter.reset();
	mod_lfo.reset();
}
void 
VoiceBoard::setFrequency(float frequency)
{
    key_pitch.setValue(frequency);
}

void 
VoiceBoard::setVelocity(float velocity)
{
	vel.setValue( velocity );
}
