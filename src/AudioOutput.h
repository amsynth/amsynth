/*
 *  AudioOutput.h
 *
 *  Copyright (c) 2001-2020 Nick Dowell
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

#include "main.h"
#include "types.h"

#include <thread>


class GenericOutput
{
public:
	virtual ~GenericOutput () {}

	virtual	int			init			() = 0;
	
	virtual	bool		Start 			() = 0;
	virtual	void		Stop			() = 0;;
};

class AudioOutput : public GenericOutput
{
public:
	virtual ~AudioOutput();

	bool	Start	() override;
	void	Stop	() override;

	int 	init	() override;

	void	ThreadAction	();

private:
	int channels = 0;
	class AudioDriver *driver = nullptr;
	float *buffer = nullptr;
	bool shouldStop = false;
	std::thread thread;
};

class NullAudioOutput : public GenericOutput { public:
	int  init  () override { return -1; }
	bool Start () override { return false; }
	void Stop  () override {}
};

#endif				// _AUDIO_OUTPUT_H
