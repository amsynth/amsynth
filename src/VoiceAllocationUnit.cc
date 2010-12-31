/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#include "VoiceAllocationUnit.h"
#include "VoiceBoard/VoiceBoard.h"
#include "Effects/SoftLimiter.h"
#include "Effects/revmodel.hpp"
#include "Effects/Distortion.h"

#include <iostream>
#include <math.h>
#include <cstring>
#include <assert.h>

using namespace std;

const unsigned kMaxGrainSize = 64;
const unsigned kBufferSize = 1024;

static const VoiceBoardProcessMemory s_procMem (kMaxGrainSize);

VoiceAllocationUnit::VoiceAllocationUnit ()
:	mMaxVoices (0)
,	mActiveVoices (0)
,	sustain (0)
,	mMasterVol (1.0)
,	mPitchBendRangeSemitones(2)
{
	limiter = new SoftLimiter;
	reverb = new revmodel;
	distortion = new Distortion;
	mBuffer = new float [kBufferSize * 2];

	for (int i = 0; i < 128; i++)
	{
		keyPressed[i] = 0;
		active[i] = false;
		_voices.push_back (new VoiceBoard (&s_procMem));
		_voices.back()->setFrequency ((440.0f/32.0f) * pow (2.0f, (float)((i-9.0)/12.0)));
	}

	SetSampleRate (44100);
}

VoiceAllocationUnit::~VoiceAllocationUnit	()
{
	while (_voices.size()) { delete _voices.back(); _voices.pop_back(); }
	delete limiter;
	delete reverb;
	delete distortion;
	delete [] mBuffer;
}

void
VoiceAllocationUnit::SetSampleRate	(int rate)
{
	limiter->SetSampleRate (rate);
	for (unsigned i=0; i<_voices.size(); ++i) _voices[i]->SetSampleRate (rate);
}

void
VoiceAllocationUnit::HandleMidiNoteOn(int note, float velocity)
{
	assert (note >= 0);
	assert (note < 128);
  
	keyPressed[note] = 1;
	
	if ((!mMaxVoices || (mActiveVoices < mMaxVoices)) && !active[note])
	{
		_voices[note]->reset();
		active[note]=1;
		mActiveVoices++;
	}

	_voices[note]->setVelocity(velocity);
	_voices[note]->triggerOn();
}

void
VoiceAllocationUnit::HandleMidiNoteOff(int note, float /*velocity*/)
{
	keyPressed[note] = 0;
	if (!sustain){
		_voices[note]->triggerOff();
	}
}

void
VoiceAllocationUnit::HandleMidiPitchWheel(float value)
{
	float newval = pow(2.0f, value * mPitchBendRangeSemitones / 12.0f);
	for (unsigned i=0; i<_voices.size(); i++) _voices[i]->SetPitchBend (newval);
}

void
VoiceAllocationUnit::HandleMidiAllSoundOff()
{
	for (unsigned i=0; i<_voices.size(); i++) active[i] = false;
	reverb->mute();
	mActiveVoices = 0;
	sustain = 0;
}

void
VoiceAllocationUnit::HandleMidiAllNotesOff()
{
	for (unsigned i=0; i<_voices.size(); i++) active[i] = false;
	reverb->mute();
	mActiveVoices = 0;
	sustain = 0;
}

void
VoiceAllocationUnit::HandleMidiSustainPedal(uchar value)
{
	sustain = value ? 1 : 0;
	if (!sustain) return;
	for(unsigned i=0; i<_voices.size(); i++) {
		if (!keyPressed[i]) _voices[i]->triggerOff();
	}
}


void 
VoiceAllocationUnit::purgeVoices()
{
	for (unsigned note = 0; note < _voices.size(); note++) 
		if (active[note] && (_voices[note]->isSilent())) {
			mActiveVoices--;
			active[note] = 0;
		}
}


void
VoiceAllocationUnit::Process		(float *l, float *r, unsigned nframes, int stride)
{
	if (nframes > kBufferSize) {
		this->Process(l,               r,                         kBufferSize, stride);
		this->Process(l + kBufferSize, r + kBufferSize, nframes - kBufferSize, stride);
		return;
	}
	
	float* vb = mBuffer;
	memset(vb, 0, nframes * sizeof (float));

	unsigned framesLeft = nframes, j = 0;
	while (0 < framesLeft)
	{
		int fr = (framesLeft < kMaxGrainSize) ? framesLeft : kMaxGrainSize;
		for (unsigned i=0; i<_voices.size(); i++) if (active[i]) _voices[i]->ProcessSamplesMix (vb+j, fr, mMasterVol);
		j += fr; framesLeft -= fr;
	}

	distortion->Process (vb, nframes);
	reverb->processreplace (vb, l,r, nframes, 1, stride); // mono -> stereo
	limiter->Process (l,r, nframes, stride);

	purgeVoices();
}

void
VoiceAllocationUnit::UpdateParameter	(Param param, float value)
{
	switch (param)
	{
	case kMasterVol:		mMasterVol = value;		break;
	case kReverbRoomsize:	reverb->setroomsize (value);	break;
	case kReverbDamp:		reverb->setdamp (value);	break;
	case kReverbWet:		reverb->setwet (value); reverb->setdry(1.0f-value); break;
	case kReverbWidth:		reverb->setwidth (value);	break;
	case kDistortionCrunch:	distortion->SetCrunch (value);	break;
	
	default: for (unsigned i=0; i<_voices.size(); i++) _voices[i]->UpdateParameter (param, value); break;
	}
}
