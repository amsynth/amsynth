/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#ifndef _VOICEBOARD_H
#define _VOICEBOARD_H

#include "../PresetController.h"
#include "../Preset.h"
#include "Synth--.h"
#include "ADSR.h"
#include "Amplifier.h"
#include "Oscillator.h"
#include "FValue.h"
#include "NFValue.h"
#include "Adder.h"
#include "Multiplier.h"
#include "LowPassFilter.h"
#include "Mixer.h"
#include "ProcessAndHold.h"
#include "../Preset.h"
#include "../Parameter.h"

/**
 * the VoiceBoard is what makes the nice noises... ;-)
 *
 * one VoiceBoard is a 'voice' in its own right, and play only one note at a 
 * time. the VoiceAllocationUnit decides which voices do what etc...
 **/

class VoiceBoard:public NFSource {
  public:
    VoiceBoard(int rate);
    virtual ~VoiceBoard();
    void init();
    inline float *getNFData();
    int getState();
    void triggerOn();
    void triggerOff();
    void setVelocity(float velocity);
    void setFrequency(float frequency);
    void setPreset(Preset & preset) {
	_preset = &preset;
    };
    void setPitchWheelParam(Parameter & param);
    void setPitchWheel(FSource & source);
	void reset();
  private:
    int rate;
    float *buffer;
    Preset *_preset;

	NFValue one, vel;
	FValue half, two;
	
    // pitch control section
    FSource *pitch_bend;
    FValue key_pitch;
    Multiplier freq, freq_mod_mult;
	NFValue freq_mod_mix_amount;
	Mixer freq_mod_mix;
    ProcessAndHold master_freq;

	// modulation section
	Oscillator mod_lfo;
	FValue lfo_freq;
	Multiplier mod_mult;
	Adder mod_add;
	ProcessAndHold mod_signal;
	
    // oscillator section
    Oscillator osc1, osc2;
	Multiplier osc2_freq, osc1_pw;
	FValue osc2_detune, osc2_range;
	NFValue osc_mix, osc1_pulsewidth_control, osc2_pulsewidth_control, osc1_pwm_amt;
	Mixer osc_mixer, osc1_pw_mixer;

    // filter section
	LowPassFilter filter;
	Adder filter_cutoff_final;
	Multiplier filter_cutoff, filter_env_signal;
	FValue cutoff_control, filter_env_amount;
	ADSR filter_env;
	Mixer filter_cutoff_mixer;
	NFValue filter_mod_amount;
	
    // amp section
    ADSR amp_env;
	Multiplier amp;
	Mixer amp_mixer;
	NFValue amp_mod_amount;
};

#endif
