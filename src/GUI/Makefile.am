include $(top_srcdir)/common.am

noinst_LIBRARIES = libGUI.a

libGUI_a_CPPFLAGS = $(AM_CPPFLAGS) \
    @ALSA_CFLAGS@ \
    @GTKMM_CFLAGS@ \
    @JACK_CFLAGS@ \
    @LASH_CFLAGS@ \
    @SNDFILE_CFLAGS@

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
	controllers.h \
	GUI.cc \
	GUI.h \
	gui_main.cc \
	gui_main.h \
	MIDILearnDialog.cpp \
	MIDILearnDialog.h \
	PresetControllerView.cc \
	PresetControllerView.h \
	presets_menu.cpp \
	presets_menu.h \
	Request.h
