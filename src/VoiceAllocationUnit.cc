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

using std::cout;

const int kMaxGrainSize = 64;

VoiceAllocationUnit::VoiceAllocationUnit( Config & config )
{
	this->config = &config;
	max_voices = config.polyphony;
#ifdef _DEBUG
	cout << "<VoiceAllocationUnit> new VAU created" << endl;
#endif

	limiter = new SoftLimiter (config.sample_rate);
	reverb = new revmodel;
	distortion = new Distortion;
	
	process_memory = new VoiceBoardProcessMemory (kMaxGrainSize);

	for (int i = 0; i < 128; i++) {
		keyPressed[i] = 0;
		active[i] = false;
		_voices.push_back (VoiceBoard (process_memory));
		_voices.back().SetSampleRate (config.sample_rate);
		_voices.back().setFrequency ((440.0/32.0) * pow (2.0f, (float)((i-9.0)/12.0)));
	}
  
	sustain = 0;
	mMasterVol = 1.0;
}

VoiceAllocationUnit::~VoiceAllocationUnit	()
{
	delete limiter;
	delete reverb;
	delete distortion;
	delete process_memory;
}

void
VoiceAllocationUnit::pwChange( float value )
{
	float newval = pow(2.0f,value);
	for (int i=0; i<128; i++) _voices[i].SetPitchBend (newval);
}

void
VoiceAllocationUnit::sustainOff()
{
	sustain = 0;
	for(int i=0; i<128; i++)
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
	
	if (!active[note])
		if( !max_voices || config->active_voices < max_voices )
		{
			_voices[note].reset();
			active[note]=1;
			config->active_voices++;
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
	for (int note = 0; note < 128; note++) 
		if (active[note] && (0 == _voices[note].getState()))
		{
			config->active_voices--;
			active[note] = 0;
		}
}

void
VoiceAllocationUnit::killAllVoices()
{
	int i;
	for (i=0; i<128; i++) active[i] = false;
	reverb->mute();
	config->active_voices = 0;
}

void
VoiceAllocationUnit::set_max_voices	( int voices )
{
	config->polyphony = max_voices = voices;
}

void
VoiceAllocationUnit::Process		(float *l, float *r, unsigned nframes)
{
	memset (l, 0, nframes * sizeof (float));

	int framesLeft=nframes; int j=0;
	while (0 < framesLeft)
	{
		int fr = (framesLeft < kMaxGrainSize) ? framesLeft : kMaxGrainSize;
		for (int i=0; i<128; i++) if (active[i]) _voices[i].ProcessSamplesMix (l+j, kMaxGrainSize, mMasterVol);
		j += fr; framesLeft -= fr;
	}

	distortion->Process (l, nframes);
	reverb->processreplace (l,l, l,r, nframes, 1);
	limiter->Process (l,r, nframes);
}

void
VoiceAllocationUnit::UpdateParameter	(Param param, float value)
{
	switch (param)
	{
	case kMasterVol:	mMasterVol = value;		break;
	case kReverbRoomsize:	reverb->setroomsize (value);	break;
	case kReverbDamp:		reverb->setdamp (value);	break;
	case kReverbWet:		reverb->setwet (value); reverb->setdry(1.0-value); break;
	case kReverbWidth:		reverb->setwidth (value);	break;
	case kReverbMode:		reverb->setmode (value);	break;
	case kReverbDry:		break;
	
	case kDistortionDrive:	break;
	case kDistortionCrunch:	distortion->SetCrunch (value);	break;
	
	default:		for (int i=0; i<128; i++) _voices[i].UpdateParameter (param, value);
				break;
	}
}
