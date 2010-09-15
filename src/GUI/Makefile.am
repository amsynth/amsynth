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
	
bin2c: bin2c.c
	$(CC) -o $@ $<

libGUI_a_SOURCES = \
	bitmap_button.c \
	bitmap_button.h \
	bitmap_knob.c \
	bitmap_knob.h \
	bitmap_popup.c \
	bitmap_popup.h \
	editor_pane.c \
	editor_pane.h \
	amsynth_logo.h \
	ConfigDialog.cpp \
	ConfigDialog.h \
	ControllerMapDialog.cc \
	ControllerMapDialog.h \
	controllers.h \
	GUI.cc \
	GUI.h \
	gui_main.cc \
	gui_main.h \
	PrefixHeader.h \
	PresetControllerView.cc \
	PresetControllerView.h \
	Request.h

