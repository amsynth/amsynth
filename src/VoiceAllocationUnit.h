/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#ifndef _VOICEALLOCATIONUNIT_H
#define _VOICEALLOCATIONUNIT_H

#include <vector>

#include "PresetController.h"
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

	void	SetSampleRate		(int) {};
  
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
  std::vector<VoiceBoard>	_voices;
  Preset *_preset;
  PresetController *_presetController;
  Config *config;
  SoftLimiter	*limiter;
  revmodel		*reverb;
  Distortion	*distortion;

	float	mMasterVol;
};

#endif
