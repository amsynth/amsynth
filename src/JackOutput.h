/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#ifndef _JACK_OUTPUT_H
#define _JACK_OUTPUT_H

#include <jack/jack.h>

#include "AudioOutput.h"
#include "VoiceBoard/Synth--.h"
#include "Config.h"

class JackOutput : public GenericOutput {

public:
  JackOutput();
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
  void stop();
  void startRecording();
  void stopRecording();
  void setOutputFile( string file )
  { wavoutfile = file; };
  string getOutputFile()
  { return wavoutfile; };
  void setConfig( Config & config );
private:
  int running;
  int channels;
  Config *config;
  string wavoutfile;
  int recording;
	int 		bufsize, srate;
};

#endif				// _AUDIO_OUTPUT_H
