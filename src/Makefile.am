bin_PROGRAMS = amSynth

SUBDIRS = drivers VoiceBoard GUI Effects

amsynth_core_sources = \
    Parameter.cc Parameter.h \
    Preset.cc Preset.h \
    PresetController.cc PresetController.h \
    VoiceAllocationUnit.cc VoiceAllocationUnit.h \
    TuningMap.cc TuningMap.h \
    Config.cc Config.h \
    controls.c controls.h \
    UpdateListener.h

amsynth_core_libs = \
    VoiceBoard/libVoiceBoard.a \
    Effects/libEffects.a

# http://www.gnu.org/software/libtool/manual/autoconf/Defining-Directories.html
amSynth_CPPFLAGS = $(AM_CPPFLAGS) -DENABLE_BINRELOC -DDEFAULT_PREFIX='"$(prefix)"' \
    @ALSA_CFLAGS@ \
    @GTKMM_CFLAGS@ \
    @JACK_CFLAGS@ \
    @LASH_CFLAGS@ \
    @SNDFILE_CFLAGS@

amSynth_SOURCES = \
	$(amsynth_core_sources) \
	main.cc main.h \
	lash.c lash.h \
	AudioOutput.cc AudioOutput.h \
	JackOutput.cc JackOutput.h \
	MidiController.cc MidiController.h \
	Thread.h \
	binreloc.h binreloc.c

amSynth_LDADD = \
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

install-exec-hook:
if ENABLE_REALTIME
	chown root $(DESTDIR)$(bindir)/amSynth
	chmod   +s $(DESTDIR)$(bindir)/amSynth
endif

