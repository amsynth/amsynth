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

class VoiceAllocationUnit;

class GenericOutput
{
public:
	virtual	void		setInput	(VoiceAllocationUnit* src)
				{ mInput = src; }

	virtual	int		init		( Config & config )	= 0;
	virtual	void		run 		( )			= 0;
	virtual	void		stop		( )			= 0;
	
	virtual	bool		canRecord	( )	{ return false; }
	virtual	void		startRecording	( )			{;}
	virtual	void		stopRecording	( )			{;};
	virtual	void		setOutputFile	( string file )		= 0;
	virtual	string		getOutputFile	( )			= 0;


	virtual	const char*	getTitle	( )	{ return "amSynth"; };

protected:
	VoiceAllocationUnit*	mInput;
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
	bool	canRecord	( )	{ return true; };
#else
	bool	canRecord	( )	{ return false; };
#endif
	void	startRecording	( );
	void 	stopRecording	( );
	void 	setOutputFile	( string file )	{ wavoutfile = file; };
  	string 	getOutputFile	( )	{ return wavoutfile; };
	int 	init		( Config & config );

private:
  int running;
  int channels;
  Config *config;
  AudioInterface out;
  string wavoutfile;
  int recording;
  float	*buffer;
#ifdef with_sndfile
  SNDFILE *sndfile;
  SF_INFO sf_info;
#endif
};

#endif				// _AUDIO_OUTPUT_H
