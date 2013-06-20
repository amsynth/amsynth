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
	banks/BriansBank01.amSynth.bank \
	banks/BriansBank02.amSynth.bank \
	banks/BriansBank03.amSynth.bank \
	banks/BriansBank04.amSynth.bank \
	banks/BriansBank05.amSynth.bank \
	banks/BriansBank06.amSynth.bank \
	banks/BriansBank07.amSynth.bank \
	banks/BriansBank08.amSynth.bank \
	banks/BriansBank09.amSynth.bank \
	banks/BriansBank10.amSynth.bank \
	banks/BriansBank11.amSynth.bank

if BUILD_LV2
amsynth_lv2dir = $(libdir)/lv2/amsynth.lv2
dist_amsynth_lv2_DATA = \
	amsynth.lv2/manifest.ttl \
	amsynth.lv2/amsynth.ttl \
	amsynth.lv2/amsynth_factory.bank.ttl \
	amsynth.lv2/BriansBank01.amSynth.bank.ttl \
	amsynth.lv2/BriansBank02.amSynth.bank.ttl \
	amsynth.lv2/BriansBank03.amSynth.bank.ttl \
	amsynth.lv2/BriansBank04.amSynth.bank.ttl \
	amsynth.lv2/BriansBank05.amSynth.bank.ttl \
	amsynth.lv2/BriansBank06.amSynth.bank.ttl \
	amsynth.lv2/BriansBank07.amSynth.bank.ttl \
	amsynth.lv2/BriansBank08.amSynth.bank.ttl \
	amsynth.lv2/BriansBank09.amSynth.bank.ttl \
	amsynth.lv2/BriansBank10.amSynth.bank.ttl \
	amsynth.lv2/BriansBank11.amSynth.bank.ttl
endif
