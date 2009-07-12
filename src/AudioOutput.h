/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#ifndef _AUDIO_OUTPUT_H
#define _AUDIO_OUTPUT_H

#ifdef with_sndfile
#include <sndfile.h>
#endif

#include "drivers/AudioInterface.h"
#include "Config.h"
#include "Thread.h"

class VoiceAllocationUnit;

class GenericOutput
{
public:
	virtual ~GenericOutput () {}

	virtual	void		setInput	(VoiceAllocationUnit* src)
				{ mInput = src; }

	virtual	int		init		( Config & config )	= 0;
	
	virtual	bool		Start 			() = 0;
	virtual	void		Stop			() = 0;
	
	virtual	bool		canRecord	( )	{ return false; }
	virtual	void		startRecording	( )			{;}
	virtual	void		stopRecording	( )			{;};
	virtual	void		setOutputFile	( string file )	{}
	virtual	string		getOutputFile	( ) { return ""; }


	virtual	const char*	getTitle	( )	{ return "amSynth"; };

protected:
	VoiceAllocationUnit*	mInput;
};

class AudioOutput : public GenericOutput, public Thread
{
public:
	AudioOutput();
	virtual ~AudioOutput();

	bool	Start	();
	void	Stop	();

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

	void	ThreadAction	();

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

class NullAudioOutput : public GenericOutput { public:
	virtual	int  init  ( Config & config ) { return -1; }
	virtual	bool Start () { return -1; }
	virtual	void Stop  () {}
};

#endif				// _AUDIO_OUTPUT_H
