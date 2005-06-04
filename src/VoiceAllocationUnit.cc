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

static VoiceBoardProcessMemory* process_memory;

using namespace std;

const int kMaxGrainSize = 64;

VoiceAllocationUnit::VoiceAllocationUnit ()
:	mMaxVoices (0)
,	mActiveVoices (0)
,	sustain (0)
,	mMasterVol (1.0)
{
	process_memory = new VoiceBoardProcessMemory (kMaxGrainSize);
	limiter = new SoftLimiter;
	reverb = new revmodel;
	distortion = new Distortion;

	for (int i = 0; i < 128; i++)
	{
		keyPressed[i] = 0;
		active[i] = false;
		_voices.push_back (VoiceBoard (process_memory));
		_voices.back().setFrequency ((440.0f/32.0f) * pow (2.0f, (float)((i-9.0)/12.0)));
	}

	SetSampleRate (44100);
}

VoiceAllocationUnit::~VoiceAllocationUnit	()
{
	delete limiter;
	delete reverb;
	delete distortion;
	delete process_memory;
}

void
VoiceAllocationUnit::SetSampleRate	(int rate)
{
	limiter->SetSampleRate (rate);
	for (unsigned i=0; i<_voices.size(); ++i) _voices[i].SetSampleRate (rate);
}

void
VoiceAllocationUnit::pwChange( float value )
{
	float newval = pow(2.0f,value);
	for (unsigned i=0; i<_voices.size(); i++) _voices[i].SetPitchBend (newval);
}

void
VoiceAllocationUnit::sustainOff()
{
	sustain = 0;
	for(unsigned i=0; i<_voices.size(); i++)
		if (!keyPressed[i]) 
			_voices[i].triggerOff();
}

void
VoiceAllocationUnit::noteOn(int note, float velocity)
{
#ifdef _DEBUG
	cout << "<VoiceAllocationUnit> noteOn(note:" << note 
	<< " vel:" << velocity << ")" << endl;
#endif
  
	purgeVoices ();
	
	keyPressed[note] = 1;
	
	if ((!mMaxVoices || (mActiveVoices < mMaxVoices)) && !active[note])
	{
		_voices[note].reset();
		active[note]=1;
		mActiveVoices++;
	}

	_voices[note].setVelocity(velocity);
	_voices[note].triggerOn();
}

void 
VoiceAllocationUnit::noteOff(int note)
{
#ifdef _DEBUG
	cout << "<VoiceAllocationUnit> noteOff(note:" << note << ")" << endl;
#endif
	keyPressed[note] = 0;
	if (!sustain){
		_voices[note].triggerOff();
	}
}

void 
VoiceAllocationUnit::purgeVoices()
{
	for (unsigned note = 0; note < _voices.size(); note++) 
		if (active[note] && (0 == _voices[note].getState()))
		{
			mActiveVoices--;
			active[note] = 0;
		}
}

void
VoiceAllocationUnit::killAllVoices()
{
	for (unsigned i=0; i<_voices.size(); i++) active[i] = false;
	reverb->mute();
	mActiveVoices = 0;
}

void
VoiceAllocationUnit::Process		(float *l, float *r, unsigned nframes, int stride)
{
	memset (l, 0, nframes * sizeof (float));

	int framesLeft=nframes; int j=0;
	while (0 < framesLeft)
	{
		int fr = (framesLeft < kMaxGrainSize) ? framesLeft : kMaxGrainSize;
		for (unsigned i=0; i<_voices.size(); i++) if (active[i]) _voices[i].ProcessSamplesMix (l+j, fr, mMasterVol);
		j += fr; framesLeft -= fr;
	}

	distortion->Process (l, nframes);
	reverb->processreplace (l, l,r, nframes, 1, 1);
	limiter->Process (l,r, nframes);
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
	
	default: for (unsigned i=0; i<_voices.size(); i++) _voices[i].UpdateParameter (param, value); break;
	}
}
