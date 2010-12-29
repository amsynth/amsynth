#ifndef _amsynth_lash_h
#define _amsynth_lash_h

#ifdef __cplusplus
extern "C" {
#endif

void amsynth_lash_process_args(int *argc, char ***argv);
void amsynth_lash_init();
void amsynth_lash_poll_events();
void amsynth_lash_set_jack_client_name(const char *name);
void amsynth_lash_set_alsa_client_id(unsigned char id);

#ifdef __cplusplus
}
#endif

#endif

