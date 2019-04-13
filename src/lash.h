/*
 *  lash.h
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

#ifndef _amsynth_lash_h
#define _amsynth_lash_h

#ifdef __cplusplus
extern "C" {
#endif

void amsynth_lash_process_args(int *argc, char ***argv);
void amsynth_lash_init(void);
void amsynth_lash_poll_events(void);
void amsynth_lash_set_jack_client_name(const char *name);
void amsynth_lash_set_alsa_client_id(unsigned char id);

#ifdef __cplusplus
}
#endif

#endif

