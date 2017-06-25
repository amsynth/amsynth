#  amsynth Makefile.am
#
#  Copyright (C) 2001-2015 Nick Dowell
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
AM_CPPFLAGS = -DDATADIR='"$(datadir)"' -DPKGDATADIR='"$(pkgdatadir)"' -DPACKAGE_LOCALEDIR=\""$(datadir)/locale"\" @CPPFLAGS@ -fPIC

ACLOCAL_AMFLAGS = -I m4

SUBDIRS = po

libcore_sources = \
	src/Configuration.cpp \
	src/Configuration.h \
	src/controls.h \
	src/midi.h \
	src/MidiController.cc \
	src/MidiController.h \
	src/Parameter.cc \
	src/Parameter.h \
	src/Preset.cc \
	src/Preset.h \
	src/PresetController.cc \
	src/PresetController.h \
	src/Thread.h \
	src/types.h \
	src/UpdateListener.h

libdsp_sources = \
	src/Effects/Distortion.cc \
	src/Effects/Distortion.h \
	src/Effects/SoftLimiter.cc \
	src/Effects/SoftLimiter.h \
	src/freeverb/allpass.cpp \
	src/freeverb/allpass.hpp \
	src/freeverb/comb.cpp \
	src/freeverb/comb.hpp \
	src/freeverb/denormals.h \
	src/freeverb/revmodel.cpp \
	src/freeverb/revmodel.hpp \
	src/freeverb/tuning.h \
	src/Synthesizer.cpp \
	src/Synthesizer.h \
	src/TuningMap.cc \
	src/TuningMap.h \
	src/VoiceAllocationUnit.cc \
	src/VoiceAllocationUnit.h \
	src/VoiceBoard/ADSR.cc \
	src/VoiceBoard/ADSR.h \
	src/VoiceBoard/LowPassFilter.cc \
	src/VoiceBoard/LowPassFilter.h \
	src/VoiceBoard/Oscillator.cc \
	src/VoiceBoard/Oscillator.h \
	src/VoiceBoard/Synth--.h \
	src/VoiceBoard/VoiceBoard.cc \
	src/VoiceBoard/VoiceBoard.h

libgui_sources = \
	src/GUI/bitmap_button.c \
	src/GUI/bitmap_button.h \
	src/GUI/bitmap_knob.c \
	src/GUI/bitmap_knob.h \
	src/GUI/bitmap_popup.c \
	src/GUI/bitmap_popup.h \
	src/GUI/controllers.h \
	src/GUI/editor_menus.cpp \
	src/GUI/editor_menus.h \
	src/GUI/editor_pane.c \
	src/GUI/editor_pane.h


#
# amsynth standalone
#

bin_PROGRAMS = amsynth

amsynth_SOURCES = \
	$(libcore_sources) \
	$(libdsp_sources) \
	src/AudioOutput.cc \
	src/AudioOutput.h \
	src/drivers/ALSAAudioDriver.cc \
	src/drivers/ALSAAudioDriver.h \
	src/drivers/ALSAMidiDriver.cc \
	src/drivers/ALSAMidiDriver.h \
	src/drivers/ALSAmmapAudioDriver.cc \
	src/drivers/ALSAmmapAudioDriver.h \
	src/drivers/AudioDriver.h \
	src/drivers/MidiDriver.h \
	src/drivers/OSSAudioDriver.cc \
	src/drivers/OSSAudioDriver.h \
	src/drivers/OSSMidiDriver.cc \
	src/drivers/OSSMidiDriver.h \
	src/gettext.h \
	src/JackOutput.cc \
	src/JackOutput.h \
	src/lash.c \
	src/lash.h \
	src/main.h \
	src/main.cc

amsynth_CPPFLAGS = $(AM_CPPFLAGS) @ALSA_CFLAGS@ @JACK_CFLAGS@ @LASH_CFLAGS@ @SNDFILE_CFLAGS@ @GTK_CFLAGS@

amsynth_LDADD = @ALSA_LIBS@ @JACK_LIBS@ @LASH_LIBS@ @SNDFILE_LIBS@ @LIBS@

if BUILD_GUI
amsynth_SOURCES += $(libgui_sources) \
	src/GUI/ConfigDialog.cpp \
	src/GUI/ConfigDialog.h \
	src/GUI/gui_main.cc \
	src/GUI/gui_main.h \
	src/GUI/MainMenu.cpp \
	src/GUI/MainMenu.h \
	src/GUI/MainWindow.cpp \
	src/GUI/MainWindow.h \
	src/GUI/MIDILearnDialog.cpp \
	src/GUI/MIDILearnDialog.h \
	src/GUI/PresetControllerView.cc \
	src/GUI/PresetControllerView.h
amsynth_LDADD += @GTK_LIBS@
endif

if DARWIN
amsynth_SOURCES += \
	src/drivers/CoreAudio.cc \
	src/drivers/CoreAudio.h
amsynth_LDFLAGS = $(AM_LDFLAGS) -framework CoreFoundation -framework CoreAudio -framework CoreMIDI
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
amsynth_dssi_gtk_LDADD = @DSSI_LIBS@ @GTK_LIBS@ @LIBLO_LIBS@ -lstdc++

endif


#
# LV2
#

if BUILD_LV2

amsynth_lv2dir = $(libdir)/lv2/amsynth.lv2
noinst_LTLIBRARIES += amsynth_lv2.la
amsynth_lv2_la_SOURCES = src/amsynth_lv2.cpp src/amsynth_lv2.h $(libcore_sources) $(libdsp_sources) \
	include/lv2/lv2plug.in/ns/lv2core/lv2.h \
	include/lv2/lv2plug.in/ns/lv2core/lv2_util.h \
	include/lv2/lv2plug.in/ns/ext/time/time.h \
	include/lv2/lv2plug.in/ns/ext/state/state.h \
	include/lv2/lv2plug.in/ns/ext/uri-map/uri-map.h \
	include/lv2/lv2plug.in/ns/ext/parameters/parameters.h \
	include/lv2/lv2plug.in/ns/ext/midi/midi.h \
	include/lv2/lv2plug.in/ns/ext/patch/patch.h \
	include/lv2/lv2plug.in/ns/ext/event/event.h \
	include/lv2/lv2plug.in/ns/ext/event/event-helpers.h \
	include/lv2/lv2plug.in/ns/ext/options/options.h \
	include/lv2/lv2plug.in/ns/ext/atom/atom.h \
	include/lv2/lv2plug.in/ns/ext/atom/util.h \
	include/lv2/lv2plug.in/ns/ext/atom/forge.h \
	include/lv2/lv2plug.in/ns/ext/port-props/port-props.h \
	include/lv2/lv2plug.in/ns/ext/log/log.h \
	include/lv2/lv2plug.in/ns/ext/log/logger.h \
	include/lv2/lv2plug.in/ns/ext/morph/morph.h \
	include/lv2/lv2plug.in/ns/ext/worker/worker.h \
	include/lv2/lv2plug.in/ns/ext/resize-port/resize-port.h \
	include/lv2/lv2plug.in/ns/ext/presets/presets.h \
	include/lv2/lv2plug.in/ns/ext/urid/urid.h \
	include/lv2/lv2plug.in/ns/ext/buf-size/buf-size.h \
	include/lv2/lv2plug.in/ns/ext/data-access/data-access.h \
	include/lv2/lv2plug.in/ns/ext/port-groups/port-groups.h \
	include/lv2/lv2plug.in/ns/ext/instance-access/instance-access.h \
	include/lv2/lv2plug.in/ns/ext/dynmanifest/dynmanifest.h \
	include/lv2/lv2plug.in/ns/extensions/ui/ui.h \
	include/lv2/lv2plug.in/ns/extensions/units/units.h
amsynth_lv2_la_CPPFLAGS = $(AM_CPPFLAGS) -I"$(top_srcdir)/include" @LV2_CFLAGS@
amsynth_lv2_la_LDFLAGS = -rpath $(amsynth_lv2dir) -avoid-version -module -export-symbols-regex "lv2_descriptor" -disable-static

noinst_LTLIBRARIES += amsynth_lv2_gtk.la
amsynth_lv2_gtk_la_SOURCES = src/amsynth_lv2_ui_gtk.cpp $(libcore_sources) $(libgui_sources)
amsynth_lv2_gtk_la_CPPFLAGS = $(AM_CPPFLAGS) -I"$(top_srcdir)/include" @LV2_CFLAGS@
amsynth_lv2_gtk_la_LDFLAGS = -rpath $(amsynth_lv2dir) -avoid-version -module -export-symbols-regex "lv2ui_descriptor" -disable-static
amsynth_lv2_gtk_la_LIBADD = @LV2_LIBS@

dist_amsynth_lv2_DATA = \
	data/amsynth.lv2/manifest.ttl \
	data/amsynth.lv2/amsynth.ttl \
	data/amsynth.lv2/amsynth_factory.bank.ttl \
	data/amsynth.lv2/BriansBank01.amSynth.bank.ttl \
	data/amsynth.lv2/BriansBank02.amSynth.bank.ttl \
	data/amsynth.lv2/BriansBank03.amSynth.bank.ttl \
	data/amsynth.lv2/BriansBank04.amSynth.bank.ttl \
	data/amsynth.lv2/BriansBank05.amSynth.bank.ttl \
	data/amsynth.lv2/BriansBank06.amSynth.bank.ttl \
	data/amsynth.lv2/BriansBank07.amSynth.bank.ttl \
	data/amsynth.lv2/BriansBank08.amSynth.bank.ttl \
	data/amsynth.lv2/BriansBank09.amSynth.bank.ttl \
	data/amsynth.lv2/BriansBank10.amSynth.bank.ttl \
	data/amsynth.lv2/BriansBank11.amSynth.bank.ttl \
	data/amsynth.lv2/BriansBank12.amSynth.bank.ttl \
	data/amsynth.lv2/BriansBank13.amSynth.bank.ttl \
	data/amsynth.lv2/BriansBank14.amSynth.bank.ttl \
	data/amsynth.lv2/BriansBank15.amSynth.bank.ttl \
	data/amsynth.lv2/BriansBank16.amSynth.bank.ttl \
	data/amsynth.lv2/BriansBank17.amSynth.bank.ttl \
	data/amsynth.lv2/BriansBank18.amSynth.bank.ttl \
	data/amsynth.lv2/BriansBank19.amSynth.bank.ttl \
	data/amsynth.lv2/BriansBank20.amSynth.bank.ttl

endif


#
# VST
#

if BUILD_VST

vstdir=$(libdir)/vst
noinst_LTLIBRARIES += amsynth_vst.la
amsynth_vst_la_SOURCES = src/amsynth_vst.cpp $(libcore_sources) $(libdsp_sources) include/vestige/aeffectx.h
amsynth_vst_la_CPPFLAGS = $(AM_CPPFLAGS) -I"$(top_srcdir)/include"
amsynth_vst_la_LDFLAGS = -rpath $(vstdir) -avoid-version -module -export-symbols-regex "VSTPluginMain|main"

if BUILD_GUI
amsynth_vst_la_SOURCES += $(libgui_sources)
amsynth_vst_la_CPPFLAGS += @GTK_CFLAGS@
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
appdata_DATA = $(appdata_in_files:.appdata.xml.in=.appdata.xml)
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
dist_banks_DATA = \
	data/banks/amsynth_factory.bank \
	data/banks/BriansBank01.amSynth.bank \
	data/banks/BriansBank02.amSynth.bank \
	data/banks/BriansBank03.amSynth.bank \
	data/banks/BriansBank04.amSynth.bank \
	data/banks/BriansBank05.amSynth.bank \
	data/banks/BriansBank06.amSynth.bank \
	data/banks/BriansBank07.amSynth.bank \
	data/banks/BriansBank08.amSynth.bank \
	data/banks/BriansBank09.amSynth.bank \
	data/banks/BriansBank10.amSynth.bank \
	data/banks/BriansBank11.amSynth.bank \
	data/banks/BriansBank12.amSynth.bank \
	data/banks/BriansBank13.amSynth.bank \
	data/banks/BriansBank14.amSynth.bank \
	data/banks/BriansBank15.amSynth.bank \
	data/banks/BriansBank16.amSynth.bank \
	data/banks/BriansBank17.amSynth.bank \
	data/banks/BriansBank18.amSynth.bank \
	data/banks/BriansBank19.amSynth.bank \
	data/banks/BriansBank20.amSynth.bank


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
