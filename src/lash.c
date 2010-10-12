#include <stdio.h>
#include <stdlib.h>

#ifdef with_lash
#include <lash/lash.h>
#endif

#include "lash.h"
#include "main.h"

#ifdef with_lash
static lash_client_t *lash_client = NULL;
#endif

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

void amsynth_lash_init(int *argc, char ***argv)
{
#ifdef with_lash
	lash_client = lash_init(
			lash_extract_args(argc, argv),
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
#endif
}

void amsynth_lash_set_jack_client_name(const char *name)
{
#ifdef with_lash
	if (lash_client) {
		lash_jack_client_name(lash_client, name);
		lash_event_t *event = lash_event_new_with_type(LASH_Client_Name);
		lash_event_set_string(event, name);
		lash_send_event(lash_client, event);
	}
#endif
}

void amsynth_lash_set_alsa_client_id(unsigned char id)
{
#ifdef with_lash
	if (lash_client)
		lash_alsa_client_id(lash_client, id);
#endif
}

void amsynth_lash_poll_events()
{
#ifdef with_lash
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
		}

		lash_event_destroy(event);
	}

	if (quit)
		exit(0);
#endif
}

