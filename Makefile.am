SUBDIRS = src skel skins

EXTRA_DIST = amsynth.png

icondir = $(datadir)/pixmaps
icon_DATA = amsynth.png

desktopdir = $(datadir)/applications
desktop_in_files = amsynth.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

banksdir = $(pkgdatadir)/banks
banks_DATA = \
	banks/amsynth_factory.bank \
	banks/BriansBank1.amSynth.bank \
	banks/BriansBank2.amSynth.bank \
	banks/BriansBank3.amSynth.bank \
	banks/BriansBank4.amSynth.bank \
	banks/BriansBank5.amSynth.bank


if BUILD_LV2
amsynth_lv2dir = $(libdir)/lv2/amsynth.lv2
dist_amsynth_lv2_DATA = \
	amsynth.lv2/manifest.ttl \
	amsynth.lv2/amsynth.ttl \
	amsynth.lv2/factory_preset_artic.ttl \
	amsynth.lv2/factory_preset_bubblebath.ttl \
	amsynth.lv2/factory_preset_caves.ttl \
	amsynth.lv2/factory_preset_church_organ.ttl \
	amsynth.lv2/factory_preset_cosmonic_2.ttl \
	amsynth.lv2/factory_preset_cosmonic.ttl \
	amsynth.lv2/factory_preset_deeck.ttl \
	amsynth.lv2/factory_preset_derren_1.ttl \
	amsynth.lv2/factory_preset_dirty_pulsating_bass.ttl \
	amsynth.lv2/factory_preset_electric_organ_1.ttl \
	amsynth.lv2/factory_preset_encounter.ttl \
	amsynth.lv2/factory_preset_flangey_bass_1.ttl \
	amsynth.lv2/factory_preset_formant_synth_lead.ttl \
	amsynth.lv2/factory_preset_glassic.ttl \
	amsynth.lv2/factory_preset_lazy_moon_lead.ttl \
	amsynth.lv2/factory_preset_my_friend.ttl \
	amsynth.lv2/factory_preset_periodic_junk.ttl \
	amsynth.lv2/factory_preset_reed_organ.ttl \
	amsynth.lv2/factory_preset_sexy_boy.ttl \
	amsynth.lv2/factory_preset_soft_simple.ttl \
	amsynth.lv2/factory_preset_sweet_bass.ttl \
	amsynth.lv2/factory_preset_synth_strings_1.ttl \
	amsynth.lv2/factory_preset_vibratio.ttl \
	amsynth.lv2/factory_preset_violin_1.ttl \
	amsynth.lv2/factory_preset_wave.ttl \
	amsynth.lv2/factory_preset_xylophone.ttl
endif
