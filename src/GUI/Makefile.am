noinst_LIBRARIES = libGUI.a

INCLUDES = $(DEPS_CFLAGS)

if USE_PRECOMPILED_HEADER
PREFIX_HEADER = PrefixHeader.h
BUILT_SOURCES = $(PREFIX_HEADER).gch
CLEANFILES = $(BUILT_SOURCES)
AM_CXXFLAGS=$(CXXFLAGS) -include $(PREFIX_HEADER)
endif

%.gch: %
	$(CXXCOMPILE) -c $<

libGUI_a_SOURCES = \
	amsynth_logo.h \
	ConfigDialog.cpp \
	ConfigDialog.h \
	ControllerMapDialog.cc \
	ControllerMapDialog.h \
	controllers.h \
	EditorPanel.cc \
	EditorPanel.h \
	GUI.cc \
	GUI.h \
	gui_main.cc \
	gui_main.h \
	Knob.cc \
	Knob.h \
	knob_med.h \
	ParameterKnob.cc \
	ParameterKnob.h \
	ParameterSwitch.cc \
	ParameterSwitch.h \
	ParameterView.h \
	PrefixHeader.h \
	PresetControllerView.cc \
	PresetControllerView.h \
	RadioButtonParameterView.cc \
	RadioButtonParameterView.h \
	Request.h

