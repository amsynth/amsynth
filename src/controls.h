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
	kLFODepth,
	kLFOWaveform,
	
	kOsc2Octave,
	kOscMix,
	
	kFreqModAmount,
	kFilterModAmount,
	kAmpModAmount,
	
	kOsc1PWMAmount,
	
	kOscMixRingMod,
	
	kOsc1Pulsewidth,
	kOsc2Pulsewidth,
	
	kReverbRoomsize,
	kReverbDamp,
	kReverbWet,
	kReverbDry,
	kReverbWidth,
	kReverbMode,
	
	kDistortionDrive,
	kDistortionCrunch,
	
	kOsc2Sync,
	
	kControls_End
};

#endif
