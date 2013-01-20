/*
 *  VoiceAllocationUnit.h
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

#ifndef _VOICEALLOCATIONUNIT_H
#define _VOICEALLOCATIONUNIT_H

#include <vector>

#include "UpdateListener.h"
#include "MidiController.h"
#include "TuningMap.h"

class VoiceBoard;
class SoftLimiter;
class revmodel;
class Distortion;

class VoiceAllocationUnit : public UpdateListener, public MidiEventHandler
{
public:
			VoiceAllocationUnit		();
	virtual	~VoiceAllocationUnit	();

	void	UpdateParameter		(Param, float);

	void	SetSampleRate		(int);
	
	virtual void HandleMidiNoteOn(int note, float velocity);
	virtual void HandleMidiNoteOff(int note, float velocity);
	virtual void HandleMidiPitchWheel(float value);
	virtual void HandleMidiAllSoundOff();
	virtual void HandleMidiAllNotesOff();
	virtual void HandleMidiSustainPedal(uchar value);

	void	SetMaxVoices	(int voices) { mMaxVoices = voices; }
	int		GetMaxVoices	() { return mMaxVoices; }
	int		GetActiveVoices	() { return mActiveVoices; }
	
	void	setPitchBendRangeSemitones(float range) { mPitchBendRangeSemitones = range; }
	void	setKeyboardMode(KeyboardMode);

	// processing with stride (interleaved) is not functional yet!!!
	void	Process			(float *l, float *r, unsigned nframes, int stride=1);

	double	noteToPitch		(int note) const;
	int		loadScale		(const string & sclFileName);
	int		loadKeyMap		(const string & kbmFileName);
	void	defaultTuning	();

private:

	int		mMaxVoices;
	int 	mActiveVoices;

	float	mPortamentoTime;
	bool	keyPressed[128], sustain;
	bool	active[128];
	
	unsigned	_keyboardMode;
	unsigned	_keyPresses[128];
	unsigned	_keyPressCounter;
	
	std::vector<VoiceBoard*>	_voices;
	
	SoftLimiter	*limiter;
	revmodel	*reverb;
	Distortion	*distortion;
	
	float	*mBuffer;

	float	mMasterVol;
	float	mPitchBendRangeSemitones;
	float	mLastNoteFrequency;
	float   mLastPitchBendValue;
	float   mNextPitchBendValue;

	TuningMap	tuningMap;
};

#endif
