/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "VoiceBoard.h"

VoiceBoard::VoiceBoard(int rate):
	// call object constructors with parameters
	mod_lfo_real(rate),
	osc1(rate), 
	osc2(rate), 
	filter(rate), 
	filter_env(rate), 
	amp_env(rate)
{
	this->rate = rate;
}

VoiceBoard::~VoiceBoard()
{
}

Parameter &
VoiceBoard::parameter( string name )
{
	return _preset->getParameter( name );
}

void
VoiceBoard::init()
{
	/*
	 * LFO
	 */
	lfo_freq.setParameter( parameter("lfo_freq") );
	mod_lfo_real.setInput( lfo_freq );
	mod_lfo_real.setWaveform( parameter("lfo_waveform") );
	
	mod_lfo.setInput( mod_lfo_real );
	
	/*
	 * pitch control section
	 */
	master_freq.setLFO( mod_lfo );
	master_freq.setPitchBend( *pitch_bend );
	master_freq.setKeyPitch( key_pitch );
	master_freq.setModAmount( parameter("freq_mod_amount") );

	/* 
	 * oscillator section
	 */
	osc1_pulsewidth_control.setParameter( parameter("osc1_pulsewidth") );
	
	osc1.setPulseWidth( osc1_pulsewidth_control );
	osc1.setWaveform( parameter("osc1_waveform") );
	osc1.setInput( master_freq );
	
	osc1.setSync( parameter("osc2_sync"), osc2 );

	osc2_freq.addInput( master_freq );
	osc2_detune.setParameter( parameter("osc2_detune") );
	osc2_freq.addInput( osc2_detune );
	osc2_range.setParameter( parameter("osc2_range") );
	osc2_freq.addInput( osc2_range );

	osc2.setWaveform( parameter("osc2_waveform") );
	osc2_pulsewidth_control.setParameter( parameter("osc2_pulsewidth") );
	osc2.setPulseWidth( osc2_pulsewidth_control );
	osc2.setInput( osc2_freq );

	osc_mix.setParameter( parameter("osc_mix") );
	osc_mixer.setControl( osc_mix );
	osc_mixer.setInput1( osc1 );
	osc_mixer.setInput2( osc2 );
	osc_mixer.setMode( parameter("osc_mix_mode") );

	/*
	 * filter section
	 */
	filter_env.setAttack( parameter("filter_attack") );
	filter_env.setDecay( parameter("filter_decay") );
	filter_env.setSustain( parameter("filter_sustain") );
	filter_env.setRelease( parameter("filter_release") );
	
	filter_control.setLFO( mod_lfo );
	filter_control.setEnvelope( filter_env );
	filter_control.setKeyPitch( key_pitch );
	filter_control.setModAmount( parameter("filter_mod_amount") );
	filter_control.setEnvAmount( parameter("filter_env_amount") );
	filter_control.setCutoffControl( parameter("filter_cutoff") );
	
	filter.setCFreq( filter_control );
	filter.setResonance( parameter("filter_resonance") );
	filter.setInput( osc_mixer );

	/* 
	 * amp section
	 */
	amp_env.setAttack( parameter("amp_attack") );
	amp_env.setDecay( parameter("amp_decay") );
	amp_env.setSustain( parameter("amp_sustain") );
	amp_env.setRelease( parameter("amp_release") );
	
	amp.setInput( filter );
	amp.setLFO( mod_lfo );
	amp.setEnvelope( amp_env );
	amp.setModAmount( parameter("amp_mod_amount") );
}

float *VoiceBoard::getNFData()
{
	mod_lfo.process();
	// note the order:
	filter_control.process();
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
	mod_lfo_real.reset();
}
void 
VoiceBoard::setFrequency(float frequency)
{
	key_pitch.setValue( frequency );
}

void 
VoiceBoard::setVelocity(float velocity)
{
	filter_control.setVelocity( velocity );
	amp.setVelocity( velocity );
}
