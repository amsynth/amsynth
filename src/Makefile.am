bin_PROGRAMS = amSynth

amSynth_SOURCES = \
		main.cc \
		AudioOutput.cc \
		MidiController.cc \
		Parameter.cc \
		Preset.cc \
		PresetController.cc \
		VoiceAllocationUnit.cc \
		Config.cc

amSynth_CXXFLAGS = `gtkmm-config --cflags`

SUBDIRS = drivers VoiceBoard GUI

amSynth_LDADD = drivers/libdrivers.a VoiceBoard/libVoiceBoard.a GUI/libGUI.a

LIBS = -lstdc++ -lm -lstdc++ -ldl `gtkmm-config --libs gtk-- gthread`
