/*
 *  JackOutput.h
 *
 *  Copyright (c) 2001-2012 Nick Dowell
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

#ifndef _JACK_OUTPUT_H
#define _JACK_OUTPUT_H

#if HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef WITH_JACK
#include <jack/jack.h>
#endif

#include "AudioOutput.h"
#include "Config.h"

class JackOutput : public GenericOutput {

public:

	JackOutput();
	
	int			init		( Config & config );// returns 0 on success
	bool		Start		();
	void		Stop		();
	
	string		get_error_msg	( )		{ return error_msg; };
	
    static bool autoconnect;

#ifdef WITH_JACK
	static int process(jack_nframes_t nframes, void *arg);
#endif
private:
	string	error_msg;
#ifdef WITH_JACK
	jack_port_t 	*l_port, *r_port, *m_port;
	jack_client_t 	*client;
#endif
};

#endif				// _JACK_OUTPUT_H
