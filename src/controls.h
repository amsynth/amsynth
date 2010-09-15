#ifndef _CONTROLS_H
#define _CONTROLS_H

enum Param 
{
	kAmpAttack,
	kAmpDecay,
	kAmpSustain,
	kAmpRelease,
	
	kOsc1Waveform,
	
	kFilterAttack,
	kFilterDecay,
	kFilterSustain,
	kFilterRelease,
	kFilterResonance,
	kFilterEnvAmount,
	kFilterCutoff,
	
	kOsc2Detune,
	kOsc2Waveform,
	
	kMasterVol,
	
	kLFOFreq,
	kLFOWaveform,
	
	kOsc2Octave,
	kOscMix,
	
	kFreqModAmount,
	kFilterModAmount,
	kAmpModAmount,
	
	kOscMixRingMod,
	
	kOsc1Pulsewidth,
	kOsc2Pulsewidth,
	
	kReverbRoomsize,
	kReverbDamp,
	kReverbWet,
	kReverbWidth,
	
	kDistortionCrunch,
	
	kOsc2Sync,
	
	kControls_End
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
