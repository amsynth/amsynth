/*
 *  AudioOutput.h
 *
 *  Copyright (c) 2001-2015 Nick Dowell
 *
 *  This file is part of amsynth.
 *
 *  amsynth is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  amsynth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with amsynth.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _AUDIO_OUTPUT_H
#define _AUDIO_OUTPUT_H

#include "Configuration.h"
#include "Thread.h"
#include "main.h"
#include "types.h"

#ifdef with_sndfile
#include <sndfile.h>
#endif

#include <string>

using std::string;


typedef void (* AudioCallback)(
	float *buffer_l, float *buffer_r, unsigned num_frames, int stride,
	amsynth_midi_event_t *events, unsigned event_count);


class GenericOutput
{
public:
	virtual ~GenericOutput () {}

	virtual void		setAudioCallback(AudioCallback callback) { mAudioCallback = callback; }

	virtual	int			init			(Configuration &config)	= 0;
	
	virtual	bool		Start 			() = 0;
	virtual	void		Stop			() = 0;
	
	virtual	bool		canRecord	( )	{ return false; }
	virtual	void		startRecording	( )			{;}
	virtual	void		stopRecording	( )			{;};
	virtual	void		setOutputFile	( string /*file*/ )	{}
	virtual	string		getOutputFile	( ) { return ""; }

protected:
	AudioCallback mAudioCallback;
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
	int 	init			( Configuration &config );

	void	ThreadAction	();

private:
  int running;
  int channels;
  Configuration *config;
  class AudioDriver *driver;
  string wavoutfile;
  int recording;
  float	*buffer;
#ifdef with_sndfile
  SNDFILE *sndfile;
  SF_INFO sf_info;
#endif
};

class NullAudioOutput : public GenericOutput { public:
	virtual	int  init  ( Configuration& ) { return -1; }
	virtual	bool Start () { return -1; }
	virtual	void Stop  () {}
};

#endif				// _AUDIO_OUTPUT_H
