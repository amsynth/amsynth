bin_PROGRAMS = amSynth.bin amSynth.ptest

INCLUDES = `gtkmm-config --cflags` `gtk-config --cflags`

amSynth_bin_SOURCES = \
		main.cc \
		AudioOutput.cc \
		JackOutput.cc \
		MidiController.cc \
		Parameter.cc \
		Preset.cc \
		PresetController.cc \
		VoiceAllocationUnit.cc \
		Config.cc

amSynth_ptest_SOURCES = ptest.cc Parameter.cc Preset.cc PresetController.cc \
		VoiceAllocationUnit.cc Config.cc

SUBDIRS = drivers VoiceBoard GUI

amSynth_bin_LDADD = drivers/libdrivers.a VoiceBoard/libVoiceBoard.a GUI/libGUI.a
amSynth_ptest_LDADD = drivers/libdrivers.a VoiceBoard/libVoiceBoard.a

LIBS = -lstdc++ -lm -lsndfile `gtkmm-config --libs gtk--`
