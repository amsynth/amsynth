/* amSynth
 * (c) 2001-2004 Nick Dowell
 */

#ifndef _VOICEALLOCATIONUNIT_H
#define _VOICEALLOCATIONUNIT_H

#include "PresetController.h"
#include "UpdateListener.h"
#include "Config.h"

#include <pthread.h>

class VoiceBoard;
class SoftLimiter;
class Reverb;
class Distortion;

class VoiceAllocationUnit : public UpdateListener
{
public:
  VoiceAllocationUnit( Config & config );
  virtual ~VoiceAllocationUnit();

	void	UpdateParameter		(Param, float);
  
  void	AllocateMemory	(int nFrames);
  
  void noteOn(int note, float velocity);
  void noteOff(int note);
  void pwChange( float value );
  void setPresetController(PresetController & p_c)
  {_presetController = &p_c; };
  void setPreset(Preset & preset);
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
  VoiceBoard *_voices[128];
  Preset *_preset;
  PresetController *_presetController;
  Config *config;
  SoftLimiter	*limiter;
  Reverb	*reverb;
  Distortion	*distortion;

	float	mMasterVol;
};

#endif
