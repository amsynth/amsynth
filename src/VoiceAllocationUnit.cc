/* amSynth
 * (c) 2001-2004 Nick Dowell
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
	
	AllocateMemory (config.buffer_size);
	for (int i = 0; i < 128; i++) {
		keyPressed[i] = 0;
		active[i] = false;
		_voices[i] = new VoiceBoard(process_memory);
		_voices[i]->SetSampleRate(config.sample_rate);
		// voices are initialised in setPreset() below...
	}
  
	sustain = 0;
	mMasterVol = 1.0;
}

void
VoiceAllocationUnit::AllocateMemory (int nFrames)
{
	if (process_memory==NULL) delete process_memory;
	process_memory = new VoiceBoardProcessMemory (nFrames);
}

VoiceAllocationUnit::~VoiceAllocationUnit	()
{
	for (int i=0; i<128; i++) delete _voices[i];
	delete limiter;
	delete reverb;
	delete distortion;
}

void
VoiceAllocationUnit::setPreset( Preset & preset )
{
	_preset = &preset;

	// now we can initialise the voices
	for( int i=0; i<128; i++ )
		_voices[i]->setFrequency( (440.0/32.0) * pow(2,((i-9.0)/12.0)) );
};

void
VoiceAllocationUnit::pwChange( float value )
{
	float newval = pow(2,value);
	for (int i=0; i<128; i++) _voices[i]->SetPitchBend (newval);
}

void
VoiceAllocationUnit::sustainOff()
{
	sustain = 0;
	for(int i=0; i<128; i++)
		if( _voices[i] && (!keyPressed[i]) ) 
			_voices[i]->triggerOff();
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
			_voices[note]->reset();
			active[note]=1;
			config->active_voices++;
		}

	_voices[note]->setVelocity(velocity);
	_voices[note]->triggerOn();
}

void 
VoiceAllocationUnit::noteOff(int note)
{
#ifdef _DEBUG
	cout << "<VoiceAllocationUnit> noteOff(note:" << note << ")" << endl;
#endif
	keyPressed[note] = 0;
	if (!sustain){
		if (_voices[note])
			_voices[note]->triggerOff();
	}
}

void 
VoiceAllocationUnit::purgeVoices()
{
#ifdef _DEBUG
	int purged = 0;
	int active = 0;
#endif
  
	for (int note = 0; note < 128; note++) {
		if (active[note]) {
			if ( _voices[note]->getState()==0 ) {
#ifdef _DEBUG
				purged++;
#endif
				config->active_voices--;
				active[note] = 0;
			}
#ifdef _DEBUG
			else active++;
#endif
		}
	}
#ifdef _DEBUG
	cout << "<VoiceAllocationUnit> removed " << purged << " voice(s) from mixer, "
	<< active << " voice(s) still connected" << endl;
#endif
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

	for (unsigned j=0; j<nframes; j+=64)
		for (int i=0; i<128; i++) if (active[i])
			_voices[i]->ProcessSamplesMix (l+j, 64, mMasterVol);

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
	
	default:		for (int i=0; i<128; i++) _voices[i]->UpdateParameter (param, value);
				break;
	}
}
