/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#ifndef _AUDIO_OUTPUT_H
#define _AUDIO_OUTPUT_H

#ifdef with_sndfile
#include <sndfile.h>
#endif

#include "VoiceBoard/Synth--.h"
#include "drivers/AudioInterface.h"
#include "Config.h"

class GenericOutput : public NFInput
{
public:
	virtual	void		setInput	( NFSource & source )	= 0;

	virtual	int		init		( Config & config )	= 0;
	virtual	void		run 		( )			= 0;
	virtual	void		stop		( )			= 0;
	
	virtual	int		canRecord	( )			= 0;
	virtual	void		startRecording	( )			= 0;
	virtual	void		stopRecording	( )			= 0;
	virtual	void		setOutputFile	( string file )		= 0;
	virtual	string		getOutputFile	( )			= 0;


	virtual	const char*	getTitle	( )	{ return "amSynth"; };
};

/**
 * @class AudioOutput
 * The AudioOutput object opens and configures an AudioInterface, and then
 * streams the signal from it's NFSource input to the output.
 * It can also record the output to a .wav file
 */
class AudioOutput : public GenericOutput {

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
#ifdef with_sndfile
	int	canRecord	( )	{ return 1; };
#else
	int	canRecord	( )	{ return 0; };
#endif
	void	startRecording	( );
	void 	stopRecording	( );
	void 	setOutputFile	( string file )	{ wavoutfile = file; };
  	string 	getOutputFile	( )	{ return wavoutfile; };
	int 	init		( Config & config );

private:
  int running;
  int channels;
  NFSource *input;
  Config *config;
  AudioInterface out;
  string wavoutfile;
  int recording;
#ifdef with_sndfile
  SNDFILE *sndfile;
  SF_INFO sf_info;
#endif
};

#endif				// _AUDIO_OUTPUT_H
