bin_SCRIPTS = amSynth
EXTRA_DIST = amSynth.in.1 amSynth.in.2
libexec_PROGRAMS = amSynth.bin
noinst_PROGRAMS = amSynth.ptest

INCLUDES = `gtkmm-config --cflags` `gtk-config --cflags`

amSynth: amSynth.in.1 amSynth.in.2
	cat amSynth.in.1 > amSynth;\
	echo PREFIX=$(prefix) >> amSynth;\
	cat amSynth.in.2 >> amSynth;\
	chmod +x amSynth

amSynth_bin_SOURCES = \
	main.cc main.h \
	AudioOutput.cc AudioOutput.h \
	JackOutput.cc JackOutput.h \
	MidiController.cc MidiController.h \
	Parameter.cc Parameter.h \
	Preset.cc Preset.h \
	PresetController.cc PresetController.h \
	VoiceAllocationUnit.cc VoiceAllocationUnit.h \
	Config.cc Config.h base.h UpdateListener.h

amSynth_ptest_SOURCES = \
	ptest.cc ptest.h Parameter.cc Preset.cc \
	PresetController.cc VoiceAllocationUnit.cc Config.cc

SUBDIRS = drivers VoiceBoard GUI

amSynth_bin_LDADD = drivers/libdrivers.a VoiceBoard/libVoiceBoard.a GUI/libGUI.a
amSynth_ptest_LDADD = drivers/libdrivers.a VoiceBoard/libVoiceBoard.a

LIBS = -lstdc++ -lm -lsndfile
amSynth_bin_LDFLAGS = `gtkmm-config --libs gtk--`
