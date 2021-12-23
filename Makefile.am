#  amsynth Makefile.am
#
#  Copyright (C) 2001-2019 Nick Dowell
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

# http://www.gnu.org/software/libtool/manual/autoconf/Defining-Directories.html
AM_CPPFLAGS = -DDATADIR='"$(datadir)"' -DPKGDATADIR='"$(pkgdatadir)"' -DPACKAGE_LOCALEDIR=\""$(datadir)/locale"\" @CPPFLAGS@ -fPIC -fvisibility=hidden

AM_CPPFLAGS += -I"$(top_srcdir)/vendor"

# https://developer.gnome.org/gtk3/stable/gtk-migrating-2-to-3.html#id-1.6.3.3
AM_CPPFLAGS += -DGDK_DISABLE_DEPRECATED -DGSEAL_ENABLE -DGTK_DISABLE_DEPRECATED -DGTK_DISABLE_SINGLE_INCLUDES

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = po

libcore_sources = \
	src/Configuration.cpp \
	src/Configuration.h \
	src/controls.h \
	src/filesystem.cpp \
	src/filesystem.h \
	src/midi.h \
	src/MidiController.cpp \
	src/MidiController.h \
	src/Parameter.cpp \
	src/Parameter.h \
	src/Preset.cpp \
	src/Preset.h \
	src/PresetController.cpp \
	src/PresetController.h \
	src/types.h \
	src/UpdateListener.h

libdsp_sources = \
	src/Effects/Distortion.cpp \
	src/Effects/Distortion.h \
	src/Effects/SoftLimiter.cpp \
	src/Effects/SoftLimiter.h \
	src/Synthesizer.cpp \
	src/Synthesizer.h \
	src/TuningMap.cpp \
	src/TuningMap.h \
	src/VoiceAllocationUnit.cpp \
	src/VoiceAllocationUnit.h \
	src/VoiceBoard/ADSR.cpp \
	src/VoiceBoard/ADSR.h \
	src/VoiceBoard/LowPassFilter.cpp \
	src/VoiceBoard/LowPassFilter.h \
	src/VoiceBoard/Oscillator.cpp \
	src/VoiceBoard/Oscillator.h \
	src/VoiceBoard/Synth--.h \
	src/VoiceBoard/VoiceBoard.cpp \
	src/VoiceBoard/VoiceBoard.h \
	vendor/freeverb/allpass.cpp \
	vendor/freeverb/allpass.hpp \
	vendor/freeverb/comb.cpp \
	vendor/freeverb/comb.hpp \
	vendor/freeverb/denormals.h \
	vendor/freeverb/revmodel.cpp \
	vendor/freeverb/revmodel.hpp \
	vendor/freeverb/tuning.h

libgui_sources = \
	src/GUI/bitmap_button.c \
	src/GUI/bitmap_button.h \
	src/GUI/bitmap_knob.c \
	src/GUI/bitmap_knob.h \
	src/GUI/bitmap_popup.c \
	src/GUI/bitmap_popup.h \
	src/GUI/editor_menus.cpp \
	src/GUI/editor_menus.h \
	src/GUI/editor_pane.c \
	src/GUI/editor_pane.h \
	src/GUI/gui_main.cpp \
	src/GUI/gui_main.h \
	vendor/seq24/controllers.h


#
# amsynth standalone
#

bin_PROGRAMS = amsynth

amsynth_SOURCES = \
	$(libcore_sources) \
	$(libdsp_sources) \
	src/AudioOutput.cpp \
	src/AudioOutput.h \
	src/drivers/ALSAAudioDriver.cpp \
	src/drivers/ALSAAudioDriver.h \
	src/drivers/ALSAMidiDriver.cpp \
	src/drivers/ALSAMidiDriver.h \
	src/drivers/ALSAmmapAudioDriver.cpp \
	src/drivers/ALSAmmapAudioDriver.h \
	src/drivers/AudioDriver.h \
	src/drivers/MidiDriver.h \
	src/drivers/OSSAudioDriver.cpp \
	src/drivers/OSSAudioDriver.h \
	src/drivers/OSSMidiDriver.cpp \
	src/drivers/OSSMidiDriver.h \
	src/gettext.h \
	src/JackOutput.cpp \
	src/JackOutput.h \
	src/lash.c \
	src/lash.h \
	src/main.h \
	src/main.cpp

amsynth_CPPFLAGS = $(AM_CPPFLAGS) @ALSA_CFLAGS@ @JACK_CFLAGS@ @LASH_CFLAGS@ @LIBLO_CFLAGS@ @GTK_CFLAGS@

amsynth_LDADD = @ALSA_LIBS@ @JACK_LIBS@ @LASH_LIBS@ @LIBLO_LIBS@ @LIBS@ -lX11

if BUILD_GUI
amsynth_SOURCES += $(libgui_sources) \
	src/GUI/ConfigDialog.cpp \
	src/GUI/ConfigDialog.h \
	src/GUI/MainMenu.cpp \
	src/GUI/MainMenu.h \
	src/GUI/MainWindow.cpp \
	src/GUI/MainWindow.h \
	src/GUI/MIDILearnDialog.cpp \
	src/GUI/MIDILearnDialog.h \
	src/GUI/PresetControllerView.cpp \
	src/GUI/PresetControllerView.h
amsynth_LDADD += @GTK_LIBS@
endif

if DARWIN
amsynth_SOURCES += \
	src/drivers/CoreAudio.cpp \
	src/drivers/CoreAudio.h
amsynth_LDFLAGS = $(AM_LDFLAGS) -framework CoreFoundation -framework CoreAudio -framework CoreMIDI
endif

if BUILD_NSM
amsynth_SOURCES += \
	src/nsm/NsmClient.cpp \
	src/nsm/NsmClient.h \
	src/nsm/NsmHandler.cpp \
	src/nsm/NsmHandler.h \
	vendor/nsm/nsm.h
endif

#
# plugins
#
# using noinst prevents the .a and .la files being installed
# plugin gets installed via custom install-exec-hook
#

noinst_LTLIBRARIES =


#
# DSSI
#

if BUILD_DSSI

dssidir = $(libdir)/dssi
noinst_LTLIBRARIES += amsynth_dssi.la
amsynth_dssi_la_SOURCES = src/amsynth_dssi.cpp src/amsynth_dssi.h $(libcore_sources) $(libdsp_sources)
amsynth_dssi_la_CPPFLAGS = $(AM_CPPFLAGS) @DSSI_CFLAGS@
amsynth_dssi_la_LDFLAGS = -rpath $(dssidir) -avoid-version -module -export-symbols-regex "dssi_descriptor" -disable-static

endif

if BUILD_DSSI_GUI

noinst_PROGRAMS = amsynth_dssi_gtk
amsynth_dssi_gtk_SOURCES = src/amsynth_dssi_gtk.cpp $(libcore_sources) $(libgui_sources)
amsynth_dssi_gtk_CPPFLAGS = $(AM_CPPFLAGS) @DSSI_CFLAGS@ @GTK_CFLAGS@ @LIBLO_CFLAGS@
amsynth_dssi_gtk_LDADD = @DSSI_LIBS@ @GTK_LIBS@ @LIBLO_LIBS@ -lstdc++ -lX11

endif


#
# LV2
#

if BUILD_LV2

amsynth_lv2dir = $(libdir)/lv2/amsynth.lv2
noinst_LTLIBRARIES += amsynth_lv2.la
amsynth_lv2_la_SOURCES = src/amsynth_lv2.cpp src/amsynth_lv2.h $(libcore_sources) $(libdsp_sources)
amsynth_lv2_la_CPPFLAGS = $(AM_CPPFLAGS) @LV2_CFLAGS@
amsynth_lv2_la_LDFLAGS = -rpath $(amsynth_lv2dir) -avoid-version -module -export-symbols-regex "lv2_descriptor" -disable-static

noinst_LTLIBRARIES += amsynth_lv2_gtk.la
amsynth_lv2_gtk_la_SOURCES = src/amsynth_lv2_ui_gtk.cpp $(libcore_sources) $(libgui_sources)
amsynth_lv2_gtk_la_CPPFLAGS = $(AM_CPPFLAGS) @LV2_CFLAGS@
amsynth_lv2_gtk_la_LDFLAGS = -rpath $(amsynth_lv2dir) -avoid-version -module -export-symbols-regex "lv2ui_descriptor" -disable-static
amsynth_lv2_gtk_la_LIBADD = @LV2_LIBS@

dist_amsynth_lv2_DATA = $(wildcard data/amsynth.lv2/*.ttl)

endif


#
# VST
#

if BUILD_VST

vstdir=$(libdir)/vst
noinst_LTLIBRARIES += amsynth_vst.la
amsynth_vst_la_SOURCES = src/amsynth_vst.cpp $(libcore_sources) $(libdsp_sources) vendor/vestige/aeffectx.h
amsynth_vst_la_LDFLAGS = -rpath $(vstdir) -avoid-version -module -export-symbols-regex "VSTPluginMain|main"

if BUILD_GUI
amsynth_vst_la_SOURCES += $(libgui_sources)
amsynth_vst_la_CPPFLAGS = $(AM_CPPFLAGS) @GTK_CFLAGS@
amsynth_vst_la_LIBADD = @GTK_LIBS@
endif

endif


#
#
#

dist_pkgdata_DATA = data/rc

EXTRA_DIST = data/icons/48x48/amsynth.png data/icons/scalable/amsynth.svg

appicon48dir=$(datadir)/icons/hicolor/48x48/apps
appicon48_DATA=data/icons/48x48/amsynth.png

appiconsvgdir=$(datadir)/icons/hicolor/scalable/apps
appiconsvg_DATA=data/icons/scalable/amsynth.svg

desktopdir = $(datadir)/applications
desktop_in_files = data/amsynth.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)

appdatadir = $(datadir)/appdata
appdata_in_files = data/amsynth.appdata.xml.in
appdata_in_files += data/dssi-amsynth-plugin.metainfo.xml.in
appdata_in_files += data/lv2-amsynth-plugin.metainfo.xml.in
appdata_in_files += data/vst-amsynth-plugin.metainfo.xml.in
appdata_DATA = $(appdata_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

EXTRA_DIST += $(appdata_in_files)

DISTCLEANFILES = $(appdata_DATA)

skinsdefaultdir="${pkgdatadir}/skins/default"
dist_skinsdefault_DATA = \
	data/skins/default/background.png \
	data/skins/default/button_simple.png \
	data/skins/default/filter_slope.png \
	data/skins/default/filter_type.png \
	data/skins/default/keybmode.png \
	data/skins/default/knob.png \
	data/skins/default/knob_boost.png \
	data/skins/default/knob_boost_cut.png \
	data/skins/default/knob_mix.png \
	data/skins/default/knob_osc_octave.png \
	data/skins/default/knob_osc_pitch.png \
	data/skins/default/knob_spot.png \
	data/skins/default/knob_width.png \
	data/skins/default/layout.ini \
	data/skins/default/osc_select.png \
	data/skins/default/portamento_modes.png \
	data/skins/default/slider_boost_1.png \
	data/skins/default/slider_boost_2.png \
	data/skins/default/waveform_lfo.png \
	data/skins/default/waveform_pics.png

banksdir = $(pkgdatadir)/banks
dist_banks_DATA = $(wildcard data/banks/*.bank)


#
# Man pages
#

if GENERATE_MAN_PAGES
SUBDIRS += man
endif


#
#
#

install-exec-hook:
if ENABLE_REALTIME
	chown root $(DESTDIR)$(bindir)/amsynth
	chmod   +s $(DESTDIR)$(bindir)/amsynth
endif
if BUILD_DSSI
	$(INSTALL_PROGRAM_ENV) $(INSTALL) -d $(DESTDIR)$(dssidir)
	$(INSTALL_PROGRAM_ENV) $(LIBTOOL) --mode=install $(INSTALL) \
		$(top_builddir)/.libs/amsynth_dssi.so $(DESTDIR)$(dssidir)/amsynth_dssi.so
endif
if BUILD_DSSI_GUI
	$(INSTALL_PROGRAM_ENV) $(INSTALL) -d $(DESTDIR)$(dssidir)/amsynth_dssi
	$(INSTALL_PROGRAM_ENV) $(LIBTOOL) --mode=install $(INSTALL) \
		$(top_builddir)/amsynth_dssi_gtk $(DESTDIR)$(dssidir)/amsynth_dssi/amsynth_dssi_gtk
endif
if BUILD_LV2
	$(INSTALL_PROGRAM_ENV) $(INSTALL) -d $(DESTDIR)$(amsynth_lv2dir)
	$(INSTALL_PROGRAM_ENV) $(LIBTOOL) --mode=install $(INSTALL) \
		$(top_builddir)/.libs/amsynth_lv2.so $(DESTDIR)$(amsynth_lv2dir)/amsynth_lv2.so
	$(INSTALL_PROGRAM_ENV) $(LIBTOOL) --mode=install $(INSTALL) \
		$(top_builddir)/.libs/amsynth_lv2_gtk.so $(DESTDIR)$(amsynth_lv2dir)/amsynth_lv2_gtk.so
endif
if BUILD_VST
	$(INSTALL_PROGRAM_ENV) $(INSTALL) -d $(DESTDIR)$(vstdir)
	$(INSTALL_PROGRAM_ENV) $(LIBTOOL) --mode=install $(INSTALL) \
		$(top_builddir)/.libs/amsynth_vst.so $(DESTDIR)$(vstdir)/amsynth_vst.so
endif


#
#
#

uninstall-hook:
	rm -rf $(DESTDIR)$(pkgdatadir)
if BUILD_DSSI
	rm  -f $(DESTDIR)$(dssidir)/amsynth_dssi.so
endif
if BUILD_DSSI_GUI
	rm -rf $(DESTDIR)$(dssidir)/amsynth_dssi
endif
if BUILD_LV2
	rm -rf $(DESTDIR)$(amsynth_lv2dir)
endif
if BUILD_VST
	rm  -f $(DESTDIR)$(vstdir)/amsynth_vst.so
endif

#
#
#

check_PROGRAMS = amsynth-tests

amsynth_tests_SOURCES = \
	$(libcore_sources) \
	$(libdsp_sources) \
	src/tests.cpp

TESTS = $(check_PROGRAMS)
