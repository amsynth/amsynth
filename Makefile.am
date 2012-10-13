SUBDIRS = src skel skins

EXTRA_DIST = amsynth.png

icondir = $(datadir)/pixmaps
icon_DATA = amsynth.png

desktopdir = $(datadir)/applications
desktop_in_files = amsynth.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

if BUILD_LV2
amsynth_lv2dir = $(libdir)/lv2/amsynth.lv2
dist_amsynth_lv2_DATA = amsynth.lv2/manifest.ttl amsynth.lv2/amsynth.ttl
endif
