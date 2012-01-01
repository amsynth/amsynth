#ifndef _CONTROLS_H
#define _CONTROLS_H

enum Param 
{
	kAmsynthParameter_AmpEnvAttack             = 1,
	kAmsynthParameter_AmpEnvDecay              = 2,
	kAmsynthParameter_AmpEnvSustain            = 3,
	kAmsynthParameter_AmpEnvRelease            = 4,
	
	kAmsynthParameter_Oscillator1Waveform      = 5,
	
	kAmsynthParameter_FilterEnvAttack          = 6,
	kAmsynthParameter_FilterEnvDecay           = 7,
	kAmsynthParameter_FilterEnvSustain         = 8,
	kAmsynthParameter_FilterEnvRelease         = 9,
	kAmsynthParameter_FilterResonance          = 10,
	kAmsynthParameter_FilterEnvAmount          = 11,
	kAmsynthParameter_FilterCutoff             = 12,
	
	kAmsynthParameter_Oscillator2Detune        = 13,
	kAmsynthParameter_Oscillator2Waveform      = 14,
	
	kAmsynthParameter_MasterVolume             = 15,
	
	kAmsynthParameter_LFOFreq                  = 16,
	kAmsynthParameter_LFOWaveform              = 17,
	
	kAmsynthParameter_Oscillator2Octave        = 18,
	kAmsynthParameter_OscillatorMix            = 19,
	
	kAmsynthParameter_LFOToOscillators         = 20,
	kAmsynthParameter_LFOToFilterCutoff        = 21,
	kAmsynthParameter_LFOToAmp                 = 22,
	
	kAmsynthParameter_OscillatorMixRingMod     = 23,
	
	kAmsynthParameter_Oscillator1Pulsewidth    = 24,
	kAmsynthParameter_Oscillator2Pulsewidth    = 25,
	
	kAmsynthParameter_ReverbRoomsize           = 26,
	kAmsynthParameter_ReverbDamp               = 27,
	kAmsynthParameter_ReverbWet                = 28,
	kAmsynthParameter_ReverbWidth              = 29,
	
	kAmsynthParameter_AmpDistortion            = 30,
	
	kAmsynthParameter_Oscillator2Sync          = 31,
	
	kAmsynthParameterCount
};

#ifdef __cplusplus
extern "C" {
#endif

const char *parameter_name_from_index (int param_index);
int parameter_index_from_name (const char *param_name);

#ifdef __cplusplus
}
#endif

#endif
