/*
 *  lash.c
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

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "lash.h"

#ifdef WITH_LASH

#include "main.h"

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>

#include <lash/lash.h>

static lash_client_t *lash_client = NULL;
static lash_args_t   *lash_args   = NULL;

static size_t fsize(FILE *stream)
{
	size_t result, pos = ftell(stream);
	fseek(stream, 0, SEEK_END);
	result = ftell(stream);
	fseek(stream, pos, SEEK_SET);
	return result;
}

static char *read_file_contents(const char *filename)
{
	FILE *stream = fopen(filename, "r");
	size_t length = fsize(stream);
	char *buffer = calloc(1, length);
	if (fread(buffer, 1, length, stream) != length) {
		perror("fread");
		free(buffer);
		buffer = NULL;
	}
	fclose(stream);
	return buffer;
}

void amsynth_lash_process_args(int *argc, char ***argv)
{
	lash_args = lash_extract_args(argc, argv);
}

void amsynth_lash_init()
{
	lash_client = lash_init(
			lash_args,
			"amsynth",
			LASH_Config_File, // save data in files
			LASH_PROTOCOL(2, 0));

	if (lash_client == NULL) {
		fprintf(stderr, "Error: failed to connect to LASH.\n");
	} else {
		lash_event_t *event = lash_event_new_with_type(LASH_Client_Name);
		lash_event_set_string(event, "amsynth");
		lash_send_event(lash_client, event);
	}
}

void amsynth_lash_set_jack_client_name(const char *name)
{
	if (lash_client) {
		lash_jack_client_name(lash_client, name);
		lash_event_t *event = lash_event_new_with_type(LASH_Client_Name);
		lash_event_set_string(event, name);
		lash_send_event(lash_client, event);
	}
}

void amsynth_lash_set_alsa_client_id(unsigned char id)
{
	if (lash_client)
		lash_alsa_client_id(lash_client, id);
}

void amsynth_lash_poll_events()
{
	if (!lash_client) {
		return;
	}

	int quit = 0;
	lash_event_t *event = NULL;

	while ((event = lash_get_event(lash_client)))
	{
		switch (lash_event_get_type(event))
		{
			case LASH_Save_File:
				{
					char *filename = NULL;
					FILE *file = NULL;

					asprintf(&filename, "%s/bank", lash_event_get_string(event));
					amsynth_save_bank(filename);
					free(filename);

					asprintf(&filename, "%s/preset_number", lash_event_get_string(event));
					file = fopen(filename, "w");
					free(filename);
					fprintf(file, "%d", amsynth_get_preset_number());
					fclose(file);

					lash_send_event(lash_client, lash_event_new_with_type(LASH_Save_File));
				}
				break;

			case LASH_Restore_File:
				{
					char *buffer, *filename = NULL;
					asprintf(&filename, "%s/bank", lash_event_get_string(event));
					amsynth_load_bank(filename);
					free(filename);

					asprintf(&filename, "%s/preset_number", lash_event_get_string(event));
					buffer = read_file_contents(filename);
					free(filename);
					int preset_number = atoi(buffer);
					free(buffer);
					amsynth_set_preset_number(preset_number);
					
					lash_send_event(lash_client, lash_event_new_with_type(LASH_Restore_File));
				}
				break;

			case LASH_Quit:
				quit = 1;
				break;

			default:
				break;
		}

		lash_event_destroy(event);
	}

	if (quit)
		exit(0);
}

#else

void amsynth_lash_process_args(int *argc, char ***argv) {}
void amsynth_lash_init() {}
void amsynth_lash_poll_events() {}
void amsynth_lash_set_jack_client_name(const char *name) {}
void amsynth_lash_set_alsa_client_id(unsigned char id) {}

#endif
