bin_PROGRAMS = amSynth

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

SUBDIRS = drivers VoiceBoard GUI

amSynth_LDADD = drivers/libdrivers.a VoiceBoard/libVoiceBoard.a GUI/libGUI.a

LIBS = -lstdc++ -lm -lsndfile -ljack `gtkmm-config --libs gtk--`
