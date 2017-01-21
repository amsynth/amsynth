/*
 *  types.h
 *
 *  Copyright (c) 2014-2016 Nick Dowell
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

#ifndef __amsynth__types__
#define __amsynth__types__

#if _WIN32
#define DEPRECATED
#else
#define DEPRECATED __attribute__((deprecated))
#endif

struct amsynth_midi_event_t {
	unsigned int offset_frames;
	unsigned int length;
	unsigned char *buffer;
};

struct amsynth_midi_cc_t {
	unsigned char channel;
	unsigned char cc;
	unsigned char value;
};

#endif
