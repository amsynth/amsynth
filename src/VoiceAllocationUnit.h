/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#ifndef _VOICEALLOCATIONUNIT_H
#define _VOICEALLOCATIONUNIT_H

#include "PresetController.h"
#include "VoiceBoard/VoiceBoard.h"
#include "VoiceBoard/Adder.h"
#include "VoiceBoard/SoftLimiter.h"
#include "VoiceBoard/FValue.h"
#include "VoiceBoard/NFValue.h"
#include "VoiceBoard/Reverb.h"
#include "VoiceBoard/Distortion.h"
#include "AudioOutput.h"
#include "VoiceAllocationUnit.h"
#include "UpdateListener.h"
#include "Config.h"

#include <pthread.h>

class VoiceAllocationUnit : public NFSource {
public:
  VoiceAllocationUnit( Config & config );
  virtual ~VoiceAllocationUnit(){};
  inline float *getNFData()
  { return limiter.getFData(); };
  void noteOn(int note, float velocity);
  void noteOff(int note);
  void pwChange( float value );
  void setPresetController(PresetController & p_c)
  {_presetController = &p_c; };
  void setPreset(Preset & preset);
  void sustainOn()
  { sustain = 1; };
  void sustainOff();
private:
  int max_voices;
  void purgeVoices();
  Adder mixer;
  SoftLimiter limiter;
  Multiplier amp;
  NFValue master_vol;
  FValue pw_val;
  NFValue zero;
  float _pitch[128];
  float outBuffer[BUF_SIZE*2];
  char keyPressed[128], sustain, connected[128];
  VoiceBoard *_voices[128];
  pthread_mutex_t voiceMutex[128];
  Preset *_preset;
  PresetController *_presetController;
  Reverb reverb;
  Distortion distortion;
  Config *config;
};

#endif

