/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#ifndef _AUDIO_OUTPUT_H
#define _AUDIO_OUTPUT_H

#include "VoiceBoard/Synth--.h"
#include "drivers/AudioInterface.h"
#include "Config.h"

/**
 * @class AudioOutput
 * The AudioOutput object opens and configures an AudioInterface, and then
 * streams the signal from it's NFSource input to the output
 */
class AudioOutput : public NFInput {

public:
  AudioOutput();
  virtual ~AudioOutput();
  /**
   * @param source the NFSource which produces the audio signal
   */
  void setInput(NFSource & source);
  /**
   * the main controller function. This function _never_ returns...
   * until the stop() method is invoked (from another thread of execution
   * obviously)
   */
  void run();
  /**
   * Stops execution 
   */
  void stop() {
    running = 0;
  };
  void setConfig( Config config );
private:
  int running;
  int channels;
  NFSource *input;
  Config *config;
  AudioInterface out;
};

#endif				// _AUDIO_OUTPUT_H
