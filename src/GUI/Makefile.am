noinst_LIBRARIES = libGUI.a

INCLUDES = @DEPS_CFLAGS@

bin2c: bin2c.c
	@CC@ -o $@ $<

# http://www.gnu.org/software/libtool/manual/autoconf/Defining-Directories.html
libGUI_a_CPPFLAGS = $(AM_CPPFLAGS) -DDEFAULT_PREFIX='"$(prefix)"'

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

