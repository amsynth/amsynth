bin_PROGRAMS = amSynth

SUBDIRS = drivers VoiceBoard GUI Effects

# http://www.gnu.org/software/libtool/manual/autoconf/Defining-Directories.html
amSynth_CPPFLAGS = $(AM_CPPFLAGS) -DENABLE_BINRELOC -DDEFAULT_PREFIX='"$(prefix)"' @LASH_CFLAGS@

amSynth_SOURCES = \
	main.cc main.h \
	lash.c lash.h \
	AudioOutput.cc AudioOutput.h \
	JackOutput.cc JackOutput.h \
	MidiController.cc MidiController.h \
	Parameter.cc Parameter.h \
	Preset.cc Preset.h \
	PresetController.cc PresetController.h \
	VoiceAllocationUnit.cc VoiceAllocationUnit.h \
	Config.cc Config.h UpdateListener.h Thread.h controls.h controls.c \
	binreloc.h binreloc.c TuningMap.h TuningMap.cc

amSynth_LDADD = \
	drivers/libdrivers.a \
	VoiceBoard/libVoiceBoard.a \
	GUI/libGUI.a \
	Effects/libEffects.a \
	-lstdc++ -lm -lpthread -lX11 @DEPS_LIBS@ @LASH_LIBS@ @LIBS@

if ENABLE_REALTIME
install-exec-hook:
	chown root $(DESTDIR)$(bindir)/amSynth
	chmod   +s $(DESTDIR)$(bindir)/amSynth
endif

