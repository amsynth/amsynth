/* amSynth
 * (c) 2001-2004 Nick Dowell
 */

#include "VoiceBoard.h"

VoiceBoard::VoiceBoard(int rate, VoiceBoardProcessMemory *mem):
	// call object constructors with parameters
	osc				(rate, mem->osc_1),
	lfo1			(rate, mem->lfo_osc_1),
	osc1			(rate, mem->osc_1),
	osc2			(rate, mem->osc_2),
	filter			(rate), 
	filter_env		(rate,mem->filter_env), 
	amp_env			(rate,mem->amp_env)
{
	this->rate = rate;
	this->mem = mem;
	osc1.SetSyncOsc (osc2);
}

void
VoiceBoard::UpdateParameter	(Param param, float value)
{
	switch (param)
	{
	case kAmpModAmount:	mAmpModAmount = (value+1.0)/2.0;break;
	case kLFOFreq:		mLFO1Freq = value; 		break;
	case kLFOWaveform:	lfo1.SetWaveform ((Oscillator::Waveform) (int)value);
				break;
	case kFreqModAmount:	mFreqModAmount=(value/2.0)+0.5;	break;
	
	case kOsc1Waveform:	osc1.SetWaveform ((Oscillator::Waveform) (int)value);
				break;
	case kOsc1Pulsewidth:	mOsc1PulseWidth = value;	break;
	case kOsc2Waveform:	osc2.SetWaveform ((Oscillator::Waveform) (int)value);
				break;
	case kOsc2Pulsewidth:	mOsc2PulseWidth = value;	break;
	case kOsc2Octave:	mOsc2Octave = value;		break;
	case kOsc2Detune:	mOsc2Detune = value;		break;
	case kOsc2Sync:		osc1.SetSync ((bool) value);	break;

	case kFilterModAmount:	mFilterModAmt = (value+1.0)/2.0;break;
	case kFilterEnvAmount:	mFilterEnvAmt = value;		break;
	case kFilterCutoff:	mFilterCutoff = value;		break;
	case kFilterResonance:	mFilterRes = value;		break;
	case kFilterAttack:	filter_env.SetAttack (value);	break;
	case kFilterDecay:	filter_env.SetDecay (value);	break;
	case kFilterSustain:	filter_env.SetSustain (value);	break;
	case kFilterRelease:	filter_env.SetRelease (value);	break;

	case kOscMixRingMod:	mRingModAmt = value;		break;
	case kOscMix:		mOsc1Vol = (1-value)/2.0;
				mOsc2Vol = (value+1)/2.0;	break;
	
	case kAmpAttack:	amp_env.SetAttack (value);	break;
	case kAmpDecay:		amp_env.SetDecay (value);	break;
	case kAmpSustain:	amp_env.SetSustain (value);	break;
	case kAmpRelease:	amp_env.SetRelease (value);	break;
		
	default: break;
	}
}

void
VoiceBoard::SetPitchBend	(float val)
{	
	mPitchBend = val;
}

void
VoiceBoard::ProcessSamplesMix	(float *buffer, int numSamples, float vol)
{
	//
	// Control Signals
	//
	float *lfo1buf = mem->lfo_osc_1;
	lfo1.ProcessSamples (lfo1buf, numSamples, mLFO1Freq, 0);

	float osc1freq = mPitchBend*mKeyPitch * ( mFreqModAmount*(lfo1buf[0]+1.0) + 1.0 - mFreqModAmount );
	float osc1pw = mOsc1PulseWidth;

	float osc2freq = osc1freq * mOsc2Detune * mOsc2Octave;
	float osc2pw = mOsc2PulseWidth;

	float env_f = *filter_env.getNFData (numSamples);
        float cutoff = mKeyPitch * env_f * mFilterEnvAmt + ( mKeyPitch * mKeyVelocity * mFilterCutoff ) * ( (lfo1buf[0]*0.5 + 0.5) * mFilterModAmt + 1-mFilterModAmt );

	//
	// VCOs
	//
	float *osc1buf = mem->osc_1;
	float *osc2buf = mem->osc_2;
	osc1.ProcessSamples (osc1buf, numSamples, osc1freq, osc1pw);
	osc2.ProcessSamples (osc2buf, numSamples, osc2freq, osc2pw);

	//
	// Osc Mix
	//
	float osc1vol = mOsc1Vol;
	float osc2vol = mOsc2Vol;
	if (mRingModAmt == 1.0) osc1vol = osc2vol = 0.0;
	for (int i=0; i<numSamples; i++)
		osc1buf[i] =
			osc1vol * osc1buf[i] +
			osc2vol * osc2buf[i] +
			mRingModAmt * osc1buf[i]*osc2buf[i];

	//
	// VCF
	//
	filter.ProcessSamples (osc1buf, numSamples, cutoff, mFilterRes);
	
	//
	// VCA
	// 
	float *ampenvbuf = amp_env.getNFData (numSamples);
	for (int i=0; i<numSamples; i++) 
		osc1buf[i] = osc1buf[i]*ampenvbuf[i]*mKeyVelocity *
			( ((lfo1buf[i]*0.5)+0.5)*mAmpModAmount + 1-mAmpModAmount);

	//
	// Copy, with overall volume
	//
	for (int i=0; i<numSamples; i++) buffer[i] += (osc1buf[i] * vol);
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
	lfo1.reset();
}
void 
VoiceBoard::setFrequency(float frequency)
{
	mKeyPitch = frequency;
}

void 
VoiceBoard::setVelocity(float velocity)
{
	mKeyVelocity = velocity;
}
