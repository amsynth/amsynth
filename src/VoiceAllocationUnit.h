/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#ifndef _VOICEALLOCATIONUNIT_H
#define _VOICEALLOCATIONUNIT_H

#include <vector>

#include "UpdateListener.h"
#include "Config.h"

class VoiceBoard;
class SoftLimiter;
class revmodel;
class Distortion;

class VoiceAllocationUnit : public UpdateListener
{
public:
			VoiceAllocationUnit		(Config& config);
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
	void		set_max_voices	( int voices );

	void	Process			(float *l, float *r, unsigned nframes);

private:
  int max_voices;
  void purgeVoices();
  float _pitch[128];
  char keyPressed[128], sustain;
  bool	active[128];
  std::vector<VoiceBoard>	_voices;
  Config *config;
  SoftLimiter	*limiter;
  revmodel		*reverb;
  Distortion	*distortion;

	float	mMasterVol;
};

#endif
