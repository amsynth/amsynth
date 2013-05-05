SUBDIRS = src skel skins

EXTRA_DIST = amsynth.png

icondir = $(datadir)/pixmaps
icon_DATA = amsynth.png

desktopdir = $(datadir)/applications
desktop_in_files = amsynth.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

banksdir = $(pkgdatadir)/banks
dist_banks_DATA = \
	banks/amsynth_factory.bank \
	banks/BriansBank1.amSynth.bank \
	banks/BriansBank2.amSynth.bank \
	banks/BriansBank3.amSynth.bank \
	banks/BriansBank4.amSynth.bank \
	banks/BriansBank5.amSynth.bank \
	banks/BriansBank6.amSynth.bank \
	banks/BriansBank7.amSynth.bank \
	banks/BriansBank8.amSynth.bank \
	banks/BriansBank9.amSynth.bank


if BUILD_LV2
amsynth_lv2dir = $(libdir)/lv2/amsynth.lv2
dist_amsynth_lv2_DATA = \
	amsynth.lv2/manifest.ttl \
	amsynth.lv2/amsynth.ttl \
	amsynth.lv2/amsynth_factory.bank.ttl \
	amsynth.lv2/BriansBank1.amSynth.bank.ttl \
	amsynth.lv2/BriansBank2.amSynth.bank.ttl \
	amsynth.lv2/BriansBank3.amSynth.bank.ttl \
	amsynth.lv2/BriansBank4.amSynth.bank.ttl \
	amsynth.lv2/BriansBank5.amSynth.bank.ttl \
	amsynth.lv2/BriansBank6.amSynth.bank.ttl \
	amsynth.lv2/BriansBank7.amSynth.bank.ttl \
	amsynth.lv2/BriansBank8.amSynth.bank.ttl \
	amsynth.lv2/BriansBank9.amSynth.bank.ttl
endif
