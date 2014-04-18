/*
 *  VoiceAllocationUnit.cc
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

#include "VoiceAllocationUnit.h"

#include "Effects/SoftLimiter.h"
#include "Effects/revmodel.hpp"
#include "Effects/Distortion.h"
#include "VoiceBoard/VoiceBoard.h"

#include <iostream>
#include <math.h>
#include <cstring>
#include <assert.h>


using namespace std;

const unsigned kBufferSize = 1024;


VoiceAllocationUnit::VoiceAllocationUnit ()
:	mMaxVoices (0)
,	mPortamentoTime (0.0f)
,	sustain (0)
,	_keyboardMode(KeyboardModePoly)
,	mMasterVol (1.0)
,	mPanGainLeft(1)
,	mPanGainRight(1)
,	mPitchBendRangeSemitones(2)
,	mLastNoteFrequency (0.0f)
,	mLastPitchBendValue(1)
,	mNextPitchBendValue(1)
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

	// Checks if the note is within the note ranges activated in the current keyboard map.
	// The above assertions guarantee the safety of this check.
	if (!tuningMap.inActiveRange(note))
		return;

	double pitch = noteToPitch(note);
	if (pitch < 0) { // unmapped key
		return;
	}
	
	keyPressed[note] = true;
	
	if (_keyboardMode == KeyboardModePoly) {

		if (mMaxVoices) {
			unsigned count = 0;
			for (int i=0; i<128; i++)
				count = count + (active[i] ? 1 : 0);
			if (count >= mMaxVoices) {
				int idx = -1;
				// strategy 1) find the oldest voice in release phase
				unsigned keyPress = _keyPressCounter + 1;
				for (int i=0; i<128; i++) {
					if (active[i] && !keyPressed[i]) {
						if (keyPress > _keyPresses[i]) {
							keyPress = _keyPresses[i];
							idx = i;
						}
					}
				}
				if (idx < 0) {
					// strategy 2) find the oldest voice
					keyPress = _keyPressCounter + 1;
					for (int i=0; i<128; i++) {
						if (active[i]) {
							if (keyPress > _keyPresses[i]) {
								keyPress = _keyPresses[i];
								idx = i;
							}
						}
					}
				}
				assert(0 <= idx && idx < 128);
				active[idx] = false;
			}
		}

		_keyPresses[note] = (++_keyPressCounter);

		if (mLastNoteFrequency > 0.0f) {
			_voices[note]->setFrequency(mLastNoteFrequency, pitch, mPortamentoTime);
		} else {
			_voices[note]->setFrequency(pitch, pitch, 0);
		}

		if (_voices[note]->isSilent())
			_voices[note]->reset();
		
		_voices[note]->setVelocity(velocity);
		_voices[note]->triggerOn();
		
		active[note] = true;
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
		voice->setFrequency(voice->getFrequency(), pitch, mPortamentoTime);
		
		if (_keyboardMode == KeyboardModeMono || previousNote == -1)
			voice->triggerOn();
		
		active[0] = true;
	}

	mLastNoteFrequency = pitch;
}

void
VoiceAllocationUnit::HandleMidiNoteOff(int note, float /*velocity*/)
{
	// No action is required if the note is outside the active range of notes.
	if (!tuningMap.inActiveRange(note))
		return;

	keyPressed[note] = false;

	if (_keyboardMode == KeyboardModePoly) {
		if (!sustain) {
			_voices[note]->triggerOff();
		}
		_keyPresses[note] = 0;
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
			voice->setFrequency(voice->getFrequency(), noteToPitch(nextNote), mPortamentoTime);
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
	mNextPitchBendValue = pow(2.0f, value * mPitchBendRangeSemitones / 12.0f);
}

void
VoiceAllocationUnit::HandleMidiPitchWheelSensitivity(uchar semitones)
{
	setPitchBendRangeSemitones(semitones);
}

void
VoiceAllocationUnit::HandleMidiAllSoundOff()
{
	resetAllVoices();
	reverb->mute();
}

void
VoiceAllocationUnit::HandleMidiAllNotesOff()
{
	resetAllVoices();
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
VoiceAllocationUnit::resetAllVoices()
{
	for (unsigned i=0; i<_voices.size(); i++) {
		active[i] = false;
		keyPressed[i] = false;
		_keyPresses[i] = 0;
		_voices[i]->reset();
	}
	_keyPressCounter = 0;
	sustain = false;
}

void
VoiceAllocationUnit::Process(unsigned nframes, std::vector<NoteEvent> events, float *l, float *r, int stride)
{
	float pitchBendValue = mLastPitchBendValue;
	float pitchBendValueEnd = mNextPitchBendValue;
	float pitchBendValueInc = (pitchBendValueEnd - pitchBendValue) / nframes;

	memset(mBuffer, 0, nframes * sizeof (float));

	std::vector<NoteEvent>::iterator event = events.begin();
	unsigned frames_left = nframes, frame_index = 0;
	while (frames_left) {
		while (event != events.end() && event->frames <= frame_index) {
			if (event->note_on) {
				this->HandleMidiNoteOn(event->note, event->velocity);
			} else {
				this->HandleMidiNoteOff(event->note, event->velocity);
			}
			++event;
		}

		if (event != events.end()) {
			assert(event->frames >= frame_index);
		}

		unsigned chunk = std::min(frames_left, (unsigned)VoiceBoard::kMaxProcessBufferSize);
		if (event != events.end() && event->frames > frame_index) {
			unsigned offset = event->frames - frame_index;
			assert(offset < frames_left);
			chunk = std::min(chunk, offset);
		}

		for (unsigned i=0; i<_voices.size(); i++) {
			if (active[i]) {
				if (_voices[i]->isSilent()) {
					active[i] = false;
				} else {
					_voices[i]->SetPitchBend(pitchBendValue);
					_voices[i]->ProcessSamplesMix(mBuffer + frame_index, chunk, mMasterVol);
				}
			}
		}

		frame_index += chunk;
		frames_left -= chunk;
		pitchBendValue = pitchBendValue + pitchBendValueInc * chunk;
	}

	distortion->Process (mBuffer, nframes);

	for (unsigned i=0; i<nframes; i++) {
		l[i * stride] = mBuffer[i] * mPanGainLeft;
		r[i * stride] = mBuffer[i] * mPanGainRight;
	}

	reverb->processmix (l, r, l, r, nframes, stride);
	limiter->Process (l,r, nframes, stride);

	mLastPitchBendValue = pitchBendValueEnd;
}

void
VoiceAllocationUnit::Process		(float *l, float *r, unsigned nframes, int stride)
{
	if (nframes > kBufferSize) {
		this->Process(l,               r,                         kBufferSize, stride);
		this->Process(l + kBufferSize, r + kBufferSize, nframes - kBufferSize, stride);
		return;
	}

	float pitchBendValue = mLastPitchBendValue;
	float pitchBendValueEnd = mNextPitchBendValue;
	float pitchBendValueInc = (pitchBendValueEnd - pitchBendValue) / nframes;

	float* vb = mBuffer;
	memset(vb, 0, nframes * sizeof (float));

	unsigned framesLeft = nframes, j = 0;
	while (0 < framesLeft) {
		int fr = std::min(framesLeft, (unsigned)VoiceBoard::kMaxProcessBufferSize);
		for (unsigned i=0; i<_voices.size(); i++) {
			if (active[i]) {
				if (_voices[i]->isSilent()) {
					active[i] = false;
				} else {
					_voices[i]->SetPitchBend (pitchBendValue);
					_voices[i]->ProcessSamplesMix (vb+j, fr, mMasterVol);
				}
			}
		}
		j += fr; framesLeft -= fr;
		pitchBendValue = pitchBendValue + pitchBendValueInc * fr;
	}

	distortion->Process (vb, nframes);

	for (unsigned i=0; i<nframes; i++) {
		l[i * stride] = vb[i] * mPanGainLeft;
		r[i * stride] = vb[i] * mPanGainRight;
	}

	reverb->processmix (l, r, l, r, nframes, stride);
	limiter->Process (l,r, nframes, stride);

	mLastPitchBendValue = pitchBendValueEnd;
}

void
VoiceAllocationUnit::setKeyboardMode(KeyboardMode keyboardMode)
{
	if (_keyboardMode != keyboardMode) {
		_keyboardMode = keyboardMode;
		resetAllVoices();
	}
}

void
VoiceAllocationUnit::UpdateParameter	(Param param, float value)
{
	switch (param) {
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
