bin_PROGRAMS = amSynth

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

SUBDIRS = drivers VoiceBoard GUI Effects

amSynth_LDADD = drivers/libdrivers.a VoiceBoard/libVoiceBoard.a GUI/libGUI.a Effects/libEffects.a -lstdc++ -lm -lpthread $(DEPS_LIBS) $(LIBS)

amSynth_CFLAGS = $(CFLAGS) -DENABLE_BINRELOC
amSynth_CXXFLAGS = $(CXXFLAGS) -DENABLE_BINRELOC

install-exec-hook:
	chown root $(DESTDIR)$(bindir)/amSynth
	chmod +s $(DESTDIR)$(bindir)/amSynth

