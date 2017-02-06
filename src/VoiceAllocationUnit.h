/*
 *  VoiceAllocationUnit.h
 *
 *  Copyright (c) 2001-2017 Nick Dowell
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

#include "UpdateListener.h"
#include "MidiController.h"
#include "TuningMap.h"

#include <stdint.h>
#include <vector>


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
	virtual void HandleMidiPitchWheelSensitivity(uchar semitones);
	virtual void HandleMidiAllSoundOff();
	virtual void HandleMidiAllNotesOff();
	virtual void HandleMidiSustainPedal(uchar value);
	virtual void HandleMidiPan(float left, float right) { mPanGainLeft = left; mPanGainRight = right; }

	void	SetMaxVoices	(int voices) { mMaxVoices = voices; }
	int		GetMaxVoices	() { return mMaxVoices; }

	float	getPitchBendRangeSemitones() { return mPitchBendRangeSemitones; }
	void	setPitchBendRangeSemitones(float range) { mPitchBendRangeSemitones = range; }
	void	setKeyboardMode(KeyboardMode);

	void	Process			(float *l, float *r, unsigned nframes, int stride=1);

	double	noteToPitch		(int note) const;
	int		loadScale		(const std::string & sclFileName);
	int		loadKeyMap		(const std::string & kbmFileName);
	void	defaultTuning	();

// private:

	void	resetAllVoices();

	int		mMaxVoices;

	float	mPortamentoTime;
	int		mPortamentoMode;
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
	float	mPanGainLeft;
	float	mPanGainRight;
	float	mPitchBendRangeSemitones;
	float	mPitchBendValue;
	float	mLastNoteFrequency;

	TuningMap	tuningMap;
};

#endif
