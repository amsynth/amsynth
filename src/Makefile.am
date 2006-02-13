bin_PROGRAMS = amSynth
noinst_PROGRAMS = amSynth.ptest

INCLUDES = $(DEPS_CFLAGS)

amSynth_SOURCES = \
	main.cc \
	AudioOutput.cc AudioOutput.h \
	JackOutput.cc JackOutput.h \
	MidiController.cc MidiController.h \
	Parameter.cc Parameter.h \
	Preset.cc Preset.h \
	PresetController.cc PresetController.h \
	VoiceAllocationUnit.cc VoiceAllocationUnit.h \
	Config.cc Config.h UpdateListener.h Thread.h controls.h \
	binreloc.h binreloc.c

amSynth_ptest_SOURCES = \
	ptest.cc Parameter.cc Preset.cc \
	PresetController.cc VoiceAllocationUnit.cc Config.cc

SUBDIRS = drivers VoiceBoard GUI Effects

amSynth_LDADD = drivers/libdrivers.a VoiceBoard/libVoiceBoard.a GUI/libGUI.a Effects/libEffects.a

amSynth_ptest_LDADD = drivers/libdrivers.a VoiceBoard/libVoiceBoard.a Effects/libEffects.a

amSynth_CFLAGS = $(CFLAGS) -DENABLE_BINRELOC
amSynth_CXXFLAGS = $(CXXFLAGS) -DENABLE_BINRELOC
amSynth_LIBS = -lstdc++ -lm -lpthread
amSynth_LDFLAGS = $(DEPS_LIBS)

install-exec-hook):
	chown root $(DESTDIR)$(bindir)/amSynth;\
	chmod +s $(DESTDIR)$(bindir)/amSynth
