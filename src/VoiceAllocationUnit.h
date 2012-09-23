/* amSynth
 * (c) 2001-2005 Nick Dowell
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

	// processing with stride (interleaved) is not functional yet!!!
	void	Process			(float *l, float *r, unsigned nframes, int stride=1);

	double	noteToPitch		(int note) const;
	int		loadScale		(const string & sclFileName);
	int		loadKeyMap		(const string & kbmFileName);
	void	defaultTuning	();

private:

	int		mMaxVoices;
	int 	mActiveVoices;

	float	mGlissandoTime;
	char	keyPressed[128], sustain;
	bool	active[128];
	std::vector<VoiceBoard*>	_voices;
	
	SoftLimiter	*limiter;
	revmodel	*reverb;
	Distortion	*distortion;
	
	float	*mBuffer;

	float	mMasterVol;
	float	mPitchBendRangeSemitones;
	float	mLastNoteFrequency;

	TuningMap	tuningMap;
};

#endif
