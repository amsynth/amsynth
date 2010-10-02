#ifndef _amsynth_main_h
#define _amsynth_main_h

#ifdef __cplusplus
extern "C" {
#endif

extern void amsynth_save_bank(const char *filename);
extern void amsynth_load_bank(const char *filename);
extern int  amsynth_get_preset_number();
extern void amsynth_set_preset_number(int preset_no);

extern void amsynth_midi_callback(unsigned timestamp, unsigned num_bytes, unsigned char *midi_data);

#ifdef __cplusplus
}
#endif

#endif

