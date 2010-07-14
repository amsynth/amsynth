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

%_png.c: %.png bin2c
	./bin2c $*_png < $< > $@

libGUI_a_SOURCES = \
	bitmap_knob.c \
	bitmap_knob.h \
	bitmap_popup.c \
	bitmap_popup.h \
	editor_pane.c \
	editor_pane.h \
	background_png.c \
	knob_png.c \
	slider_boost_1_png.c \
	slider_boost_2_png.c \
	waveform_png.c \
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
	Knob.cc \
	Knob.h \
	knob_med.h \
	PrefixHeader.h \
	PresetControllerView.cc \
	PresetControllerView.h \
	Request.h

