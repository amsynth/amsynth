/*
 *  VoiceBoard.cpp
 *
 *  Copyright (c) 2001-2021 Nick Dowell
 *
 *  This file is part of amsynth.
 *
 *  amsynth is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  amsynth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with amsynth.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "VoiceBoard.h"

#include <cassert>
#include <cmath>

#define BLEND(x0, x1, m) (((x0) * (1.f - (m))) + ((x1) * (m)))

// Low-pass filter the VCA control signal to prevent nasty clicking sounds
const float kVCALowPassFreq = 4000.0f;

const float kKeyTrackBaseFreq = 261.626f; // Middle C

enum class LFOWaveform {
	kSine,
	kSquare,
	kTriangle,
	kNoise,
	kRandomize,
	kSawtoothUp,
	kSawtoothDown
};

void
VoiceBoard::UpdateParameter	(Param param, float value)
{
	switch (param)
	{
	case kAmsynthParameter_LFOToAmp:	mAmpModAmount = (value+1.0f)/2.0f;break;
	case kAmsynthParameter_LFOFreq:		mLFO1Freq = value; 		break;
	case kAmsynthParameter_LFOWaveform: {
		switch ((LFOWaveform)(int)value) {
			case LFOWaveform::kSine:         mLFOPulseWidth = 0.0; lfo1.SetWaveform(Oscillator::Waveform::kSine);   break;
			case LFOWaveform::kSquare:       mLFOPulseWidth = 0.0; lfo1.SetWaveform(Oscillator::Waveform::kPulse);  break;
			case LFOWaveform::kTriangle:     mLFOPulseWidth = 0.0; lfo1.SetWaveform(Oscillator::Waveform::kSaw);    break;
			case LFOWaveform::kNoise:        mLFOPulseWidth = 0.0; lfo1.SetWaveform(Oscillator::Waveform::kNoise);  break;
			case LFOWaveform::kRandomize:    mLFOPulseWidth = 0.0; lfo1.SetWaveform(Oscillator::Waveform::kRandom); break;
			case LFOWaveform::kSawtoothUp:   mLFOPulseWidth = 1.0; lfo1.SetWaveform(Oscillator::Waveform::kSaw);    lfo1.setPolarity(+1.0); break;
			case LFOWaveform::kSawtoothDown: mLFOPulseWidth = 1.0; lfo1.SetWaveform(Oscillator::Waveform::kSaw);    lfo1.setPolarity(-1.0); break;
			default: assert(nullptr == "invalid LFO waveform"); break;
		}
		break;
	}
	case kAmsynthParameter_LFOToOscillators:	mFreqModAmount=(value/2.0f)+0.5f;	break;
    case kAmsynthParameter_LFOOscillatorSelect: mFreqModDestination = (int)roundf(value); break;
	
	case kAmsynthParameter_Oscillator1Waveform:	osc1.SetWaveform ((Oscillator::Waveform) (int)value);
				break;
	case kAmsynthParameter_Oscillator1Pulsewidth:	mOsc1PulseWidth = value;	break;
	case kAmsynthParameter_Oscillator2Waveform:	osc2.SetWaveform ((Oscillator::Waveform) (int)value);
				break;
	case kAmsynthParameter_Oscillator2Pulsewidth:	mOsc2PulseWidth = value;	break;
	case kAmsynthParameter_Oscillator2Octave:	mOsc2Octave = value;		break;
	case kAmsynthParameter_Oscillator2Detune:	mOsc2Detune = value;		break;
	case kAmsynthParameter_Oscillator2Pitch:	mOsc2Pitch = ::powf(2, value / 12); break;
	case kAmsynthParameter_Oscillator2Sync:		mOsc2Sync  = roundf(value) != 0.f; break;

	case kAmsynthParameter_LFOToFilterCutoff:	mFilterModAmt = (value+1.0f)/2.0f;break;
	case kAmsynthParameter_FilterEnvAmount:	mFilterEnvAmt = value;		break;
	case kAmsynthParameter_FilterCutoff:	mFilterCutoff = value;		break;
	case kAmsynthParameter_FilterResonance:	mFilterRes = value;		break;
	case kAmsynthParameter_FilterEnvAttack:	mFilterADSR.SetAttack (value);	break;
	case kAmsynthParameter_FilterEnvDecay:	mFilterADSR.SetDecay (value);	break;
	case kAmsynthParameter_FilterEnvSustain:	mFilterADSR.SetSustain (value);	break;
	case kAmsynthParameter_FilterEnvRelease:	mFilterADSR.SetRelease (value);	break;
	case kAmsynthParameter_FilterType: mFilterType = (SynthFilter::Type) (int)value; break;
	case kAmsynthParameter_FilterSlope: mFilterSlope = (SynthFilter::Slope) (int)value; break;
	case kAmsynthParameter_FilterKeyTrackAmount: mFilterKbdTrack = value; break;
	case kAmsynthParameter_FilterKeyVelocityAmount: mFilterVelSens = value; break;

	case kAmsynthParameter_OscillatorMixRingMod:	mRingModAmt = value;		break;
	case kAmsynthParameter_OscillatorMix:			mOscMix = value;			break;
	
	case kAmsynthParameter_AmpEnvAttack:			mAmpADSR.SetAttack(value);	break;
	case kAmsynthParameter_AmpEnvDecay:				mAmpADSR.SetDecay(value);	break;
	case kAmsynthParameter_AmpEnvSustain:			mAmpADSR.SetSustain(value);	break;
	case kAmsynthParameter_AmpEnvRelease:			mAmpADSR.SetRelease(value);	break;
	case kAmsynthParameter_AmpVelocityAmount: mAmpVelSens = value; break;
		
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
	assert(numSamples <= kMaxProcessBufferSize);

	if (mFrequencyDirty) {
		mFrequencyDirty = false;
		mFrequency.configure(mFrequencyStart, mFrequencyTarget, (int) (mFrequencyTime * mSampleRate));
	}

	//
	// Control Signals
	//
	float *lfo1buf = mProcessBuffers.lfo_osc_1;
	lfo1.ProcessSamples (lfo1buf, numSamples, mLFO1Freq, mLFOPulseWidth);

	const float frequency = mFrequency.nextValue();
	for (int i=1; i<numSamples; i++) { mFrequency.nextValue(); }

	float baseFreq = mPitchBend * frequency;

	float osc1freq = baseFreq;
	if (mFreqModDestination == 0 || mFreqModDestination == 1) {
		osc1freq = osc1freq * ( mFreqModAmount * (lfo1buf[0] + 1.0f) + 1.0f - mFreqModAmount );
	}
	float osc1pw = mOsc1PulseWidth;

	float osc2freq = baseFreq * mOsc2Detune * mOsc2Octave * mOsc2Pitch;
	if (mFreqModDestination == 0 || mFreqModDestination == 2) {
		osc2freq = osc2freq * ( mFreqModAmount * (lfo1buf[0] + 1.0f) + 1.0f - mFreqModAmount );
	}
	float osc2pw = mOsc2PulseWidth;

	mFilterADSR.process(mProcessBuffers.filter_env, numSamples);
	float env_f = mProcessBuffers.filter_env[numSamples - 1];
	float cutoff_base = BLEND(kKeyTrackBaseFreq, frequency, mFilterKbdTrack);
	float cutoff_vel_mult = BLEND(1.f, mKeyVelocity, mFilterVelSens);
	float cutoff_lfo_mult = (lfo1buf[0] * 0.5f + 0.5f) * mFilterModAmt + 1 - mFilterModAmt;
	float cutoff = mFilterCutoff * cutoff_base * cutoff_vel_mult * cutoff_lfo_mult;
	if (mFilterEnvAmt > 0.f) cutoff += (frequency * env_f * mFilterEnvAmt);
	else
	{
		static const float r16 = 1.f/16.f; // scale if from -16 to -1
		cutoff += cutoff * r16 * mFilterEnvAmt * env_f;
	}
	

	//
	// VCOs
	//
	float *osc1buf = mProcessBuffers.osc_1;
	float *osc2buf = mProcessBuffers.osc_2;

	bool osc2sync = mOsc2Sync;
	// previous implementation of sync had a bug causing it to only work when osc1 was set to sine or saw
	// we need to recreate that behaviour here to ensure old presets still sound the same.
	osc2sync &= (osc1.GetWaveform() == Oscillator::Waveform::kSine || osc1.GetWaveform() == Oscillator::Waveform::kSaw);
	osc2.setSyncEnabled(osc2sync);

	osc1.ProcessSamples (osc1buf, numSamples, osc1freq, osc1pw);
	osc2.ProcessSamples (osc2buf, numSamples, osc2freq, osc2pw, osc1freq);

	//
	// Osc Mix
	//
	for (int i=0; i<numSamples; i++) {
		float ringMod = mRingModAmt.tick();
		float oscMix = mOscMix.tick();
		float osc1vol = (1.F - ringMod) * (1.F - oscMix) / 2.F;
		float osc2vol = (1.F - ringMod) * (1.F + oscMix) / 2.F;
		osc1buf[i] =
			osc1vol * osc1buf[i] +
			osc2vol * osc2buf[i] +
			ringMod * osc1buf[i] * osc2buf[i];
	}

	//
	// VCF
	//
	filter.ProcessSamples (osc1buf, numSamples, cutoff, mFilterRes, mFilterType, mFilterSlope);
	
	//
	// VCA
	// 
	float *ampenvbuf = mProcessBuffers.amp_env;
	mAmpADSR.process(ampenvbuf, numSamples);
	for (int i=0; i<numSamples; i++) {
		float ampModAmount = mAmpModAmount.tick();
		const float amplitude = ampenvbuf[i] * BLEND(1.f, mKeyVelocity, mAmpVelSens.tick()) *
			( ((lfo1buf[i] * 0.5f) + 0.5f) * ampModAmount + 1 - ampModAmount);
		buffer[i] += osc1buf[i] * _vcaFilter.processSample(amplitude * mVolume.processSample(vol));
	}
}

void
VoiceBoard::SetSampleRate	(int rate)
{
	mSampleRate = rate;
	lfo1.SetSampleRate (rate);
	osc1.SetSampleRate (rate);
	osc2.SetSampleRate (rate);
	filter.SetSampleRate (rate);
	mFilterADSR.SetSampleRate(rate);
	mAmpADSR.SetSampleRate(rate);
	_vcaFilter.setCoefficients(rate, kVCALowPassFreq, IIRFilterFirstOrder::Mode::kLowPass);
}

bool 
VoiceBoard::isSilent()
{
	return mAmpADSR.getState() == 0 && _vcaFilter._z < 0.0000001;
}

void 
VoiceBoard::triggerOn(bool reset)
{
	if (reset) {
		mOscMix.reset();
		mRingModAmt.reset();
		mAmpModAmount.reset();
		mAmpVelSens.reset();
	}
	mAmpADSR.triggerOn();
	mFilterADSR.triggerOn();
}

void 
VoiceBoard::triggerOff()
{
	mAmpADSR.triggerOff();
	mFilterADSR.triggerOff();
}

void
VoiceBoard::reset()
{
	mAmpADSR.reset();
	mFilterADSR.reset();
	osc1.reset();
	osc2.reset();
	filter.reset();
	lfo1.reset();
}

void
VoiceBoard::setFrequency(float startFrequency, float targetFrequency, float time)
{
	mFrequencyStart = startFrequency;
	mFrequencyTarget = targetFrequency;
	mFrequencyTime = time;
	mFrequencyDirty = true;
}

void
VoiceBoard::setVelocity(float velocity)
{
	assert(velocity <= 1.0f);
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
