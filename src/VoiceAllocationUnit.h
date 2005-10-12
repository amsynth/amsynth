/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#ifndef _VOICEALLOCATIONUNIT_H
#define _VOICEALLOCATIONUNIT_H

#include <vector>

#include "UpdateListener.h"

class VoiceBoard;
class SoftLimiter;
class revmodel;
class Distortion;

class VoiceAllocationUnit : public UpdateListener
{
public:
			VoiceAllocationUnit		();
	virtual	~VoiceAllocationUnit	();

	void	UpdateParameter		(Param, float);

	void	SetSampleRate		(int);
  
  void noteOn(int note, float velocity);
  void noteOff(int note);
  void pwChange( float value );
  void sustainOn()
  { sustain = 1; };
  void sustainOff();
  void killAllVoices();
  
	void	SetMaxVoices	(int voices) { mMaxVoices = voices; }
	int		GetActiveVoices	() { return mActiveVoices; }

	// processing with stride (interleaved) is not functional yet!!!
	void	Process			(float *l, float *r, unsigned nframes, int stride=1);

private:
	void	purgeVoices		();

	int		mMaxVoices;
	int 	mActiveVoices;

	char	keyPressed[128], sustain;
	bool	active[128];
	std::vector<VoiceBoard*>	_voices;
	
	SoftLimiter	*limiter;
	revmodel	*reverb;
	Distortion	*distortion;

	float	mMasterVol;
};

#endif
