/*
 *  VoiceBoard.cc
 *
 *  Copyright (c) 2001-2012 Nick Dowell
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

#define BLEND(x0, x1, m) (((x0) * (1.0 - (m))) + ((x1) * (m)))

// Low-pass filter the VCA control signal to prevent nasty clicking sounds
const float kVCALowPassFreq = 4000.0f;

const float kKeyTrackBaseFreq = 261.626; // Middle C

VoiceBoard::VoiceBoard()
:	mFrequencyDirty (false)
,	mFrequencyStart (0.0)
,	mFrequencyTarget(0.0)
,	mFrequencyTime	(0.0)
,	mKeyVelocity	(1.0)
,	mPitchBend		(1.0)
,	mLFO1Freq		(0.0)
,	mFreqModAmount	(0.0)
,	mFreqModDestination(0)
,	mOsc1PulseWidth	(0.0)
,	mOsc2PulseWidth	(0.0)
,	mOsc1Vol		(1.0)
,	mOsc2Vol		(1.0)
,	mRingModAmt		(0.0)
,	mOsc2Octave		(1.0)
,	mOsc2Detune		(1.0)
,	mOsc2Sync		(false)
,	mFilterEnvAmt	(0.0)
,	mFilterModAmt	(0.0)
,	mFilterCutoff	(16.0)
,	mFilterRes		(0.0)
,	mFilterKbdTrack (0.0)
,	mFilterVelSens	(0.0)
,	filter_env		(mProcessBuffers.filter_env)
,	mAmpModAmount	(0.0)
,	mAmpVelSens		(1.0)
,	amp_env			(mProcessBuffers.amp_env)
{
}

enum { sine, square, triangle, noise, randomize, sawtooth_up, sawtooth_down };

void
VoiceBoard::UpdateParameter	(Param param, float value)
{
	switch (param)
	{
	case kAmsynthParameter_LFOToAmp:	mAmpModAmount = (value+1.0f)/2.0f;break;
	case kAmsynthParameter_LFOFreq:		mLFO1Freq = value; 		break;
	case kAmsynthParameter_LFOWaveform: {
		switch ((int)value) {
			case sine:          mLFOPulseWidth = 0.0; lfo1.SetWaveform(Oscillator::Waveform_Sine);   break;
			case square:        mLFOPulseWidth = 0.0; lfo1.SetWaveform(Oscillator::Waveform_Pulse);  break;
			case triangle:      mLFOPulseWidth = 0.0; lfo1.SetWaveform(Oscillator::Waveform_Saw);    break;
			case noise:         mLFOPulseWidth = 0.0; lfo1.SetWaveform(Oscillator::Waveform_Noise);  break;
			case randomize:     mLFOPulseWidth = 0.0; lfo1.SetWaveform(Oscillator::Waveform_Random); break;
			case sawtooth_up:   mLFOPulseWidth = 1.0; lfo1.SetWaveform(Oscillator::Waveform_Saw);    lfo1.setPolarity(+1.0); break;
			case sawtooth_down: mLFOPulseWidth = 1.0; lfo1.SetWaveform(Oscillator::Waveform_Saw);    lfo1.setPolarity(-1.0); break;
			default: assert(!"invalid LFO waveform"); break;
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
	case kAmsynthParameter_Oscillator2Pitch:	mOsc2Pitch = ::pow(2, value / 12); break;
	case kAmsynthParameter_Oscillator2Sync:		mOsc2Sync  = !!round(value); break;

	case kAmsynthParameter_LFOToFilterCutoff:	mFilterModAmt = (value+1.0f)/2.0f;break;
	case kAmsynthParameter_FilterEnvAmount:	mFilterEnvAmt = value;		break;
	case kAmsynthParameter_FilterCutoff:	mFilterCutoff = value;		break;
	case kAmsynthParameter_FilterResonance:	mFilterRes = value;		break;
	case kAmsynthParameter_FilterEnvAttack:	filter_env.SetAttack (value);	break;
	case kAmsynthParameter_FilterEnvDecay:	filter_env.SetDecay (value);	break;
	case kAmsynthParameter_FilterEnvSustain:	filter_env.SetSustain (value);	break;
	case kAmsynthParameter_FilterEnvRelease:	filter_env.SetRelease (value);	break;
	case kAmsynthParameter_FilterType: mFilterType = (SynthFilter::FilterType) (int)value; break;
	case kAmsynthParameter_FilterSlope: mFilterSlope = (SynthFilter::FilterSlope) (int)value; break;
	case kAmsynthParameter_FilterKeyTrackAmount: mFilterKbdTrack = value; break;
	case kAmsynthParameter_FilterKeyVelocityAmount: mFilterVelSens = value; break;

	case kAmsynthParameter_OscillatorMixRingMod:	mRingModAmt = value;		break;
	case kAmsynthParameter_OscillatorMix:		mOsc1Vol = (1-value)/2.0f;
				mOsc2Vol = (value+1)/2.0f;	break;
	
	case kAmsynthParameter_AmpEnvAttack:	amp_env.SetAttack (value);	break;
	case kAmsynthParameter_AmpEnvDecay:		amp_env.SetDecay (value);	break;
	case kAmsynthParameter_AmpEnvSustain:	amp_env.SetSustain (value);	break;
	case kAmsynthParameter_AmpEnvRelease:	amp_env.SetRelease (value);	break;
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
		mFrequency.configure(mFrequencyStart, mFrequencyTarget, mFrequencyTime * mSampleRate);
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

	float env_f = filter_env.getNFData(numSamples)[numSamples - 1];
	float cutoff_base = BLEND(kKeyTrackBaseFreq, frequency, mFilterKbdTrack);
	float cutoff_vel_mult = BLEND(1.0, mKeyVelocity, mFilterVelSens);
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
	osc2sync &= (osc1.GetWaveform() == Oscillator::Waveform_Sine || osc1.GetWaveform() == Oscillator::Waveform_Saw);
	osc2.setSyncEnabled(osc2sync);

	osc1.ProcessSamples (osc1buf, numSamples, osc1freq, osc1pw);
	osc2.ProcessSamples (osc2buf, numSamples, osc2freq, osc2pw, osc1freq);

	//
	// Osc Mix
	//
	float osc1vol = mOsc1Vol * (1.0 - mRingModAmt);
	float osc2vol = mOsc2Vol * (1.0 - mRingModAmt);
	for (int i=0; i<numSamples; i++)
		osc1buf[i] =
			osc1vol * osc1buf[i] +
			osc2vol * osc2buf[i] +
			mRingModAmt * osc1buf[i] * osc2buf[i];

	//
	// VCF
	//
	filter.ProcessSamples (osc1buf, numSamples, cutoff, mFilterRes, mFilterType, mFilterSlope);
	
	//
	// VCA
	// 
	float *ampenvbuf = amp_env.getNFData (numSamples);
	for (int i=0; i<numSamples; i++) {
		const float amplitude = ampenvbuf[i] * BLEND(1.0, mKeyVelocity, mAmpVelSens) *
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
	mSampleRate = rate;
	lfo1.SetSampleRate (rate);
	osc1.SetSampleRate (rate);
	osc2.SetSampleRate (rate);
	filter.SetSampleRate (rate);
	filter_env.SetSampleRate (rate);
	amp_env.SetSampleRate (rate);
	_vcaFilter.setCoefficients(rate, kVCALowPassFreq, IIRFilterFirstOrder::LowPass);
}

bool 
VoiceBoard::isSilent()
{
	return amp_env.getState() == 0 && _vcaFilter._z < 0.0000001;
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
