SUBDIRS = src skel

dist_pkgdata_DATA = amsynth-skin.zip

EXTRA_DIST = amsynth.png

icondir = $(datadir)/pixmaps
icon_DATA = amsynth.png

desktopdir = $(datadir)/applications
desktop_in_files = amsynth.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

install-exec-hook:
	update-desktop-database

