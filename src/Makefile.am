include $(top_srcdir)/common.am

bin_PROGRAMS = amsynth

SUBDIRS = drivers VoiceBoard GUI Effects

amsynth_core_sources = \
    Parameter.cc Parameter.h \
    Preset.cc Preset.h \
    PresetController.cc PresetController.h \
    VoiceAllocationUnit.cc VoiceAllocationUnit.h \
    Synthesizer.cpp Synthesizer.h \
    TuningMap.cc TuningMap.h \
    Config.cc Config.h \
    controls.h \
    midi.h \
    UpdateListener.h

amsynth_core_libs = \
    VoiceBoard/libVoiceBoard.a \
    Effects/libEffects.a

amsynth_dsp_sources = \
	Effects/allpass.cpp \
	Effects/comb.cpp \
	Effects/Distortion.cc \
	Effects/revmodel.cpp \
	Effects/SoftLimiter.cc \
	VoiceBoard/ADSR.cc \
	VoiceBoard/LowPassFilter.cc \
	VoiceBoard/Oscillator.cc \
	VoiceBoard/VoiceBoard.cc

amsynth_CPPFLAGS = $(AM_CPPFLAGS) \
    @ALSA_CFLAGS@ \
    @GTKMM_CFLAGS@ \
    @JACK_CFLAGS@ \
    @LASH_CFLAGS@ \
    @SNDFILE_CFLAGS@

amsynth_SOURCES = \
	$(amsynth_core_sources) \
	main.cc main.h \
	lash.c lash.h \
	AudioOutput.cc AudioOutput.h \
	JackOutput.cc JackOutput.h \
	MidiController.cc MidiController.h \
	Thread.h

amsynth_LDADD = \
	$(amsynth_core_libs) \
	drivers/libdrivers.a \
	GUI/libGUI.a \
    @ALSA_LIBS@ \
    @GTKMM_LIBS@ \
    @JACK_LIBS@ \
    @LASH_LIBS@ \
    @SNDFILE_LIBS@ \
	-lpthread @LIBS@

####

noinst_LTLIBRARIES =

if BUILD_DSSI
dssidir = $(libdir)/dssi
# using noinst to prevent .a and .la files being installed, plugin gets installed via custom install-exec-hook
noinst_LTLIBRARIES += amsynth_dssi.la
amsynth_dssi_la_SOURCES = $(amsynth_core_sources) $(amsynth_dsp_sources) dssi.cpp MidiController.cc
amsynth_dssi_la_CPPFLAGS = $(AM_CPPFLAGS) @DSSI_CFLAGS@
amsynth_dssi_la_LDFLAGS = -rpath $(dssidir) -avoid-version -module -export-symbols-regex "dssi_descriptor" -disable-static
endif

if BUILD_DSSI_GUI
noinst_PROGRAMS = amsynth_dssi_gtk
amsynth_dssi_gtk_SOURCES = amsynth_dssi_gui.c Parameter.cc Preset.cc PresetController.cc GUI/bitmap_button.c GUI/bitmap_knob.c GUI/bitmap_popup.c GUI/editor_pane.c GUI/presets_menu.cpp
amsynth_dssi_gtk_CPPFLAGS = $(AM_CPPFLAGS) @DSSI_CFLAGS@ @GTK_CFLAGS@ @LIBLO_CFLAGS@
amsynth_dssi_gtk_LDADD = @DSSI_LIBS@ @GTK_LIBS@ @LIBLO_LIBS@
endif

if BUILD_LV2
amsynth_lv2dir = $(libdir)/lv2/amsynth.lv2
# using noinst to prevent .a and .la files being installed, plugin gets installed via custom install-exec-hook
noinst_LTLIBRARIES += amsynth_lv2.la
amsynth_lv2_la_SOURCES = $(amsynth_core_sources) $(amsynth_dsp_sources) amsynth_lv2.cpp MidiController.cc \
	lv2/lv2plug.in/ns/lv2core/lv2.h \
	lv2/lv2plug.in/ns/extensions/units/units.h \
	lv2/lv2plug.in/ns/extensions/ui/ui.h \
	lv2/lv2plug.in/ns/ext/worker/worker.h \
	lv2/lv2plug.in/ns/ext/data-access/data-access.h \
	lv2/lv2plug.in/ns/ext/event/event-helpers.h \
	lv2/lv2plug.in/ns/ext/event/event.h \
	lv2/lv2plug.in/ns/ext/instance-access/instance-access.h \
	lv2/lv2plug.in/ns/ext/time/time.h \
	lv2/lv2plug.in/ns/ext/patch/patch.h \
	lv2/lv2plug.in/ns/ext/uri-map/uri-map.h \
	lv2/lv2plug.in/ns/ext/port-groups/port-groups.h \
	lv2/lv2plug.in/ns/ext/port-props/port-props.h \
	lv2/lv2plug.in/ns/ext/log/log.h \
	lv2/lv2plug.in/ns/ext/resize-port/resize-port.h \
	lv2/lv2plug.in/ns/ext/midi/midi.h \
	lv2/lv2plug.in/ns/ext/presets/presets.h \
	lv2/lv2plug.in/ns/ext/atom/atom.h \
	lv2/lv2plug.in/ns/ext/atom/util.h \
	lv2/lv2plug.in/ns/ext/atom/forge.h \
	lv2/lv2plug.in/ns/ext/state/state.h \
	lv2/lv2plug.in/ns/ext/urid/urid.h \
	lv2/lv2plug.in/ns/ext/dynmanifest/dynmanifest.h
amsynth_lv2_la_CPPFLAGS = $(AM_CPPFLAGS) @LV2_CFLAGS@
amsynth_lv2_la_LDFLAGS = -rpath $(amsynth_lv2dir) -avoid-version -module -export-symbols-regex "lv2_descriptor" -disable-static
noinst_LTLIBRARIES += amsynth_lv2_gtk.la
amsynth_lv2_gtk_la_SOURCES = amsynth_lv2_ui_gtk.c Parameter.cc Preset.cc PresetController.cc GUI/bitmap_button.c GUI/bitmap_knob.c GUI/bitmap_popup.c GUI/editor_pane.c GUI/presets_menu.cpp
amsynth_lv2_gtk_la_CPPFLAGS = $(AM_CPPFLAGS) @LV2_CFLAGS@
amsynth_lv2_gtk_la_LDFLAGS = -rpath $(amsynth_lv2dir) -avoid-version -module -export-symbols-regex "lv2ui_descriptor" -disable-static
amsynth_lv2_gtk_la_LIBADD = @LV2_LIBS@
endif

####

install-exec-hook:
if ENABLE_REALTIME
	chown root $(DESTDIR)$(bindir)/amsynth
	chmod   +s $(DESTDIR)$(bindir)/amsynth
endif
if BUILD_DSSI
	install -c -D -m 755 $(top_builddir)/src/.libs/amsynth_dssi.so $(DESTDIR)$(dssidir)/amsynth_dssi.so
endif
if BUILD_DSSI_GUI
	install -c -D -m 755 $(top_builddir)/src/amsynth_dssi_gtk $(DESTDIR)$(dssidir)/amsynth_dssi/amsynth_dssi_gtk
endif
if BUILD_LV2
	install -c -D -m 755 $(top_builddir)/src/.libs/amsynth_lv2.so $(DESTDIR)$(amsynth_lv2dir)/amsynth_lv2.so
	install -c -D -m 755 $(top_builddir)/src/.libs/amsynth_lv2_gtk.so $(DESTDIR)$(amsynth_lv2dir)/amsynth_lv2_gtk.so
endif

uninstall-hook:
if BUILD_DSSI
	rm -f $(DESTDIR)$(dssidir)/amsynth_dssi.so
endif
if BUILD_DSSI_GUI
	rm -rf $(DESTDIR)$(dssidir)/amsynth_dssi
endif
if BUILD_LV2
	rm -f $(DESTDIR)$(amsynth_lv2dir)/amsynth_lv2.so
	rm -f $(DESTDIR)$(amsynth_lv2dir)/amsynth_lv2_gtk.so
endif
