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

const unsigned kBufferSize = 1024;

VoiceAllocationUnit::VoiceAllocationUnit ()
:	mMaxVoices (0)
,	mActiveVoices (0)
,	mPortamentoTime (0.0f)
,	sustain (0)
,	_keyboardMode(KeyboardModePoly)
,	mMasterVol (1.0)
,	mPitchBendRangeSemitones(2)
,	mLastNoteFrequency (0.0f)
{
	limiter = new SoftLimiter;
	reverb = new revmodel;
	distortion = new Distortion;
	mBuffer = new float [kBufferSize * 2];

	for (int i = 0; i < 128; i++)
	{
		keyPressed[i] = false;
		active[i] = false;
		_voices.push_back (new VoiceBoard);
	}
	
	memset(&_keyPresses, 0, sizeof(_keyPresses));

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

	double pitch = noteToPitch(note);
	if (pitch < 0) { // unmapped key
		return;
	}
	
	keyPressed[note] = true;
	
	if (_keyboardMode == KeyboardModePoly) {
		
		if ((0 < mMaxVoices) && (mMaxVoices <= mActiveVoices))
			return;
		
		if (mLastNoteFrequency > 0.0f) {
			_voices[note]->setFrequency(mLastNoteFrequency, 0.0);
			_voices[note]->setFrequency(pitch, mPortamentoTime);
		} else {
			_voices[note]->setFrequency(pitch);
		}
		
		_voices[note]->reset();
		_voices[note]->setVelocity(velocity);
		_voices[note]->triggerOn();
		
		active[note] = true;
		mActiveVoices++;
		mLastNoteFrequency = pitch;
	}
	
	if (_keyboardMode == KeyboardModeMono || _keyboardMode == KeyboardModeLegato) {
		
		int previousNote = -1;
		unsigned keyPress = 0;
		for (int i = 0; i < 128; i++) {
			if (keyPress < _keyPresses[i]) {
				keyPress = _keyPresses[i];
				previousNote = i;
			}
		}

		_keyPresses[note] = (++_keyPressCounter);
		
		VoiceBoard *voice = _voices[0];
		
		voice->setVelocity(velocity);
		voice->setFrequency(pitch, mPortamentoTime);
		
		if (_keyboardMode == KeyboardModeMono || previousNote == -1)
			voice->triggerOn();
		
		mLastNoteFrequency = pitch;
		mActiveVoices = 1;
		active[0] = true;
	}
}

void
VoiceAllocationUnit::HandleMidiNoteOff(int note, float /*velocity*/)
{
	keyPressed[note] = false;

	if (_keyboardMode == KeyboardModePoly) {
		if (!sustain){
			_voices[note]->triggerOff();
		}
	}
	
	if (_keyboardMode == KeyboardModeMono || _keyboardMode == KeyboardModeLegato) {

		int currentNote = -1;
		unsigned keyPress = 0;
		for (int i = 0; i < 128; i++) {
			if (keyPress < _keyPresses[i]) {
				keyPress = _keyPresses[i];
				currentNote = i;
			}
		}
		
		_keyPresses[note] = 0;
		
		int nextNote = -1;
		for (int i = 0, keyPress = 0; i < 128; i++) {
			if (keyPress < _keyPresses[i]) {
				keyPress = _keyPresses[i];
				nextNote = i;
			}
		}
		
		if (!keyPress) {
			_keyPressCounter = 0;
		}
		
		if (note != currentNote) {
			return;
		}
		
		VoiceBoard *voice = _voices[0];
		
		if (0 <= nextNote) {
			double pitch = noteToPitch(nextNote);
			voice->setFrequency(pitch, mPortamentoTime);
			if (_keyboardMode == KeyboardModeMono)
				voice->triggerOn();
		} else {
			voice->triggerOff();
		}
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
	if (sustain) return;
	for(unsigned i=0; i<_voices.size(); i++) {
		if (!keyPressed[i]) _voices[i]->triggerOff();
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
	while (0 < framesLeft) {
		int fr = std::min(framesLeft, (unsigned)VoiceBoard::kMaxProcessBufferSize);
		for (unsigned i=0; i<_voices.size(); i++) {
			if (active[i]) {
				if (_voices[i]->isSilent()) {
					active[i] = false;
					mActiveVoices--;
				} else {
					_voices[i]->ProcessSamplesMix (vb+j, fr, mMasterVol);
				}
			}
		}
		j += fr; framesLeft -= fr;
	}

	distortion->Process (vb, nframes);
	reverb->processreplace (vb, l,r, nframes, 1, stride); // mono -> stereo
	limiter->Process (l,r, nframes, stride);
}

void
VoiceAllocationUnit::setKeyboardMode(KeyboardMode keyboardMode)
{
	_keyboardMode = keyboardMode;
	if (_keyboardMode == KeyboardModePoly) {
		active[0] = false; // stop the mono voice
	} else {
		// stop any voices that were started in poly mode
		for (int i = 1; i < 128; i++) {
			if (active[i]) {
				active[i] = false;
			}
		}
	}
}

void
VoiceAllocationUnit::UpdateParameter	(Param param, float value)
{
	switch (param)
	{
	case kAmsynthParameter_MasterVolume:		mMasterVol = value;		break;
	case kAmsynthParameter_ReverbRoomsize:	reverb->setroomsize (value);	break;
	case kAmsynthParameter_ReverbDamp:		reverb->setdamp (value);	break;
	case kAmsynthParameter_ReverbWet:		reverb->setwet (value); reverb->setdry(1.0f-value); break;
	case kAmsynthParameter_ReverbWidth:		reverb->setwidth (value);	break;
	case kAmsynthParameter_AmpDistortion:	distortion->SetCrunch (value);	break;
	case kAmsynthParameter_PortamentoTime: 	mPortamentoTime = value; break;
	case kAmsynthParameter_KeyboardMode:	setKeyboardMode((KeyboardMode)value); break;
	
	default: for (unsigned i=0; i<_voices.size(); i++) _voices[i]->UpdateParameter (param, value); break;
	}
}

////////////////////////////////////////////////////////////////////////////////

double
VoiceAllocationUnit::noteToPitch	(int note) const
{
	return tuningMap.noteToPitch(note);
}

int
VoiceAllocationUnit::loadScale		(const string & sclFileName)
{
	return tuningMap.loadScale(sclFileName);
}

int
VoiceAllocationUnit::loadKeyMap		(const string & kbmFileName)
{
	return tuningMap.loadKeyMap(kbmFileName);
}

void
VoiceAllocationUnit::defaultTuning	()
{
	tuningMap.defaultScale();
	tuningMap.defaultKeyMap();
}
