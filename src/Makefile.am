bin_PROGRAMS = amSynth ptest

INCLUDES = `gtkmm-config --cflags` `gtk-config --cflags`

amSynth_SOURCES = \
		main.cc \
		AudioOutput.cc \
		JackOutput.cc \
		MidiController.cc \
		Parameter.cc \
		Preset.cc \
		PresetController.cc \
		VoiceAllocationUnit.cc \
		Config.cc

ptest_SOURCES = ptest.cc Parameter.cc Preset.cc PresetController.cc \
		VoiceAllocationUnit.cc Config.cc

SUBDIRS = drivers VoiceBoard GUI

amSynth_LDADD = drivers/libdrivers.a VoiceBoard/libVoiceBoard.a GUI/libGUI.a
ptest_LDADD = drivers/libdrivers.a VoiceBoard/libVoiceBoard.a

LIBS = -lstdc++ -lm -lsndfile `gtkmm-config --libs gtk--`
