/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#include "VoiceBoard.h"

// Low-pass filter the VCA control signal to prevent nasty clicking sounds
const float kVCALowPassFreq = 4000.0f;

VoiceBoard::VoiceBoard(const VoiceBoardProcessMemory * vbpm):
	mem				(vbpm)
,	mKeyVelocity	(1.0)
,	mKeyPitch		(440.0)
,	mPitchBend		(1.0)
,	mLFO1Freq		(0.0)
,	mFreqModAmount	(0.0)
,	mOsc1PulseWidth	(0.0)
,	mOsc2PulseWidth	(0.0)
,	mOsc1Vol		(1.0)
,	mOsc2Vol		(1.0)
,	mRingModAmt		(0.0)
,	mOsc2Octave		(1.0)
,	mOsc2Detune		(1.0)
,	mFilterEnvAmt	(0.0)
,	mFilterModAmt	(0.0)
,	mFilterCutoff	(16.0)
,	mFilterRes		(0.0)
,	filter_env		(mem->filter_env)
,	mAmpModAmount	(0.0)
,	amp_env			(mem->amp_env)
{
}

void
VoiceBoard::UpdateParameter	(Param param, float value)
{
	switch (param)
	{
	case kAmpModAmount:	mAmpModAmount = (value+1.0f)/2.0f;break;
	case kLFOFreq:		mLFO1Freq = value; 		break;
	case kLFOWaveform:	lfo1.SetWaveform ((Oscillator::Waveform) (int)value);
				break;
	case kFreqModAmount:	mFreqModAmount=(value/2.0f)+0.5f;	break;
	
	case kOsc1Waveform:	osc1.SetWaveform ((Oscillator::Waveform) (int)value);
				break;
	case kOsc1Pulsewidth:	mOsc1PulseWidth = value;	break;
	case kOsc2Waveform:	osc2.SetWaveform ((Oscillator::Waveform) (int)value);
				break;
	case kOsc2Pulsewidth:	mOsc2PulseWidth = value;	break;
	case kOsc2Octave:	mOsc2Octave = value;		break;
	case kOsc2Detune:	mOsc2Detune = value;		break;
	case kOsc2Sync:		osc1.SetSync (value>0.5 ? &osc2 : 0);	break;

	case kFilterModAmount:	mFilterModAmt = (value+1.0f)/2.0f;break;
	case kFilterEnvAmount:	mFilterEnvAmt = value;		break;
	case kFilterCutoff:	mFilterCutoff = value;		break;
	case kFilterResonance:	mFilterRes = value;		break;
	case kFilterAttack:	filter_env.SetAttack (value);	break;
	case kFilterDecay:	filter_env.SetDecay (value);	break;
	case kFilterSustain:	filter_env.SetSustain (value);	break;
	case kFilterRelease:	filter_env.SetRelease (value);	break;

	case kOscMixRingMod:	mRingModAmt = value;		break;
	case kOscMix:		mOsc1Vol = (1-value)/2.0f;
				mOsc2Vol = (value+1)/2.0f;	break;
	
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

	float osc1freq = mPitchBend*mKeyPitch * ( mFreqModAmount*(lfo1buf[0]+1.0f) + 1.0f - mFreqModAmount );
	float osc1pw = mOsc1PulseWidth;

	float osc2freq = osc1freq * mOsc2Detune * mOsc2Octave;
	float osc2pw = mOsc2PulseWidth;

	float env_f = *filter_env.getNFData (numSamples);
	float cutoff = ( mKeyPitch * mKeyVelocity * mFilterCutoff ) * ( (lfo1buf[0]*0.5f + 0.5f) * mFilterModAmt + 1-mFilterModAmt );
	if (mFilterEnvAmt > 0.f) cutoff += (mKeyPitch * env_f * mFilterEnvAmt);
	else
	{
		static const float r16 = 1.f/16.f; // scale if from -16 to -1
		cutoff += cutoff * r16 * mFilterEnvAmt * env_f;
	}
	

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
	for (int i=0; i<numSamples; i++) {
		const float amplitude = ampenvbuf[i] * mKeyVelocity * 
			( ((lfo1buf[i] * 0.5f) + 0.5f) * mAmpModAmount + 1 - mAmpModAmount);
		osc1buf[i] = osc1buf[i] * _vcaFilter.processSample(amplitude);
	}

	//
	// Copy, with overall volume
	//
	for (int i=0; i<numSamples; i++) buffer[i] += (osc1buf[i] * vol);
}

void
VoiceBoard::SetSampleRate	(int rate)
{
	lfo1.SetSampleRate (rate);
	osc1.SetSampleRate (rate);
	osc2.SetSampleRate (rate);
	filter.SetSampleRate (rate);
	filter_env.SetSampleRate (rate);
	amp_env.SetSampleRate (rate);
	_vcaFilter.setCoefficients(rate, kVCALowPassFreq, IIRFilterFirstOrder::LowPass);
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

#if 0 
////////////////////////////////// profiling code

#include <stdio.h>

#define rdtscl(low) \
	__asm__ __volatile__("rdtsc" : "=a" (low) : : "edx")

int main ()
{
#define kSamples 64
	VoiceBoardProcessMemory mem (kSamples);
	VoiceBoard vb (&mem);
	float buf [kSamples];

        
	unsigned long before, after, elapsed;
#define best_time(code,tries,time) \
	        time = 0xffffffff; \
	        for (int i=0; i<tries; i++) { \
			rdtscl(before); \
			code; \
			rdtscl(after); \
			elapsed = after - before; \
			if (elapsed < time) time = elapsed; \
		}

	unsigned long base, proc;
	best_time(,16,base);
	best_time(vb.ProcessSamplesMix (buf, kSamples, 1.f),1000,proc);
	proc -= base;

	printf ("base time : %u\n", base);
	printf ("proc time : %u\n", proc);
	printf ("per sample: %u\n", proc/kSamples);
}
#endif

