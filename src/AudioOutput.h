/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#ifndef _AUDIO_OUTPUT_H
#define _AUDIO_OUTPUT_H

#ifdef with_sndfile
#include <sndfile.h>
#endif

#include "drivers/AudioInterface.h"
#include "Config.h"

class VoiceAllocationUnit;

class GenericOutput
{
public:
	virtual	void		setInput	(VoiceAllocationUnit* src)
				{ mInput = src; }

	virtual	int		init		( Config & config )	= 0;
	
	virtual	bool		Start 			() = 0;
	virtual	void		Stop			() = 0;
	
	virtual	bool		canRecord	( )	{ return false; }
	virtual	void		startRecording	( )			{;}
	virtual	void		stopRecording	( )			{;};
	virtual	void		setOutputFile	( string file )		= 0;
	virtual	string		getOutputFile	( )			= 0;


	virtual	const char*	getTitle	( )	{ return "amSynth"; };

protected:
	VoiceAllocationUnit*	mInput;
};

class PThread
{
public:
	int		Run		() { return pthread_create (&mThread, NULL, PThread::start_routine, this); }
	void	Stop	() { mShouldStop = true; }
	int		Join	() { return pthread_join (mThread, NULL); }

protected:
	// override me!
	// and make sure to call ShouldStop() periodically and return if so.
	virtual void 	ThreadAction () = 0;
	bool			ShouldStop () { return mShouldStop; }

private:
	static void* start_routine (void *arg)
	{
		PThread *self = (PThread *) arg;
		self->mShouldStop = false;
		self->ThreadAction ();
		pthread_exit (0);
	}
	pthread_t	mThread;
	bool		mShouldStop;
};


class AudioOutput : public GenericOutput, public PThread
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

#endif				// _AUDIO_OUTPUT_H
