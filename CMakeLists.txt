cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_VERBOSE_MAKEFILE true)

find_package(PkgConfig REQUIRED)

project(amsynth)

SET(CMAKE_POSITION_INDEPENDENT_CODE True)

include_directories(${PROJECT_SOURCE_DIR}/include)

add_compile_options(
        -DVERSION="dev"
        -DPACKAGE="amsynth"
        -DPACKAGE_NAME="amsynth"
        -DPACKAGE_TARNAME="amsynth"
        -DPACKAGE_STRING="amsynth dev"
        -DPACKAGE_URL="https://amsynth.github.io"
        -DPACKAGE_BUGREPORT="https://www.github.com/amsynth/amsynth/issues"
        -DPKGDATADIR="${PROJECT_SOURCE_DIR}/data"
        -DWITH_OSS
)

add_compile_options(
        -DENABLE_NLS
        -DGETTEXT_PACKAGE="amsynth"
        -DPACKAGE_LOCALEDIR="/usr/local/share/locale"
)

add_compile_options(
        -Wall
        -Werror
)

pkg_check_modules(ALSA alsa)
if (ALSA_FOUND)
    add_compile_options(-DWITH_ALSA)
    include_directories(${ALSA_INCLUDE_DIRS})
endif (ALSA_FOUND)

pkg_check_modules(JACK jack)
if (JACK_FOUND)
    add_compile_options(-DWITH_JACK -DHAVE_JACK_MIDIPORT_H -DHAVE_JACK_SESSION_H)
    include_directories(${JACK_INCLUDE_DIRS})
endif (JACK_FOUND)

pkg_check_modules(GTK gtk+-2.0)
add_compile_options(-DWITH_GUI)
include_directories(${GTK_INCLUDE_DIRS})

pkg_check_modules(LASH lash-1.0)
if (LASH_FOUND)
    add_compile_options(-DWITH_LASH)
    include_directories(${LASH_INCLUDE_DIRS})
endif (LASH_FOUND)

pkg_check_modules(LIBLO liblo)
if (LIBLO_FOUND)
    include_directories(${LIBLO_INCLUDE_DIRS})
endif (LIBLO_FOUND)

pkg_check_modules(SNDFILE sndfile)
if (SNDFILE_FOUND)
    add_compile_options(-Dwith_sndfile)
endif(SNDFILE_FOUND)

#
#
#

add_library(amsynth_core STATIC
        src/Effects/Distortion.cc
        src/Effects/Distortion.h
        src/Effects/SoftLimiter.cc
        src/Effects/SoftLimiter.h
        src/freeverb/allpass.cpp
        src/freeverb/allpass.hpp
        src/freeverb/comb.cpp
        src/freeverb/comb.hpp
        src/freeverb/denormals.h
        src/freeverb/revmodel.cpp
        src/freeverb/revmodel.hpp
        src/freeverb/tuning.h
        src/VoiceBoard/ADSR.cc
        src/VoiceBoard/ADSR.h
        src/VoiceBoard/LowPassFilter.cc
        src/VoiceBoard/LowPassFilter.h
        src/VoiceBoard/Oscillator.cc
        src/VoiceBoard/Oscillator.h
        src/VoiceBoard/Synth--.h
        src/VoiceBoard/VoiceBoard.cc
        src/VoiceBoard/VoiceBoard.h
        src/Configuration.cpp
        src/Configuration.h
        src/controls.h
        src/gettext.h
        src/lash.c
        src/lash.h
        src/midi.h
        src/MidiController.cc
        src/MidiController.h
        src/Parameter.cc
        src/Parameter.h
        src/Preset.cc
        src/Preset.h
        src/PresetController.cc
        src/PresetController.h
        src/Synthesizer.cpp
        src/Synthesizer.h
        src/Thread.h
        src/TuningMap.cc
        src/TuningMap.h
        src/types.h
        src/UpdateListener.h
        src/VoiceAllocationUnit.cc
        src/VoiceAllocationUnit.h
        )

#
# Stand-alone application
#

add_executable(amsynth
        src/drivers/ALSAAudioDriver.cc
        src/drivers/ALSAAudioDriver.h
        src/drivers/ALSAMidiDriver.cc
        src/drivers/ALSAMidiDriver.h
        src/drivers/ALSAmmapAudioDriver.cc
        src/drivers/ALSAmmapAudioDriver.h
        src/drivers/AudioDriver.h
        src/drivers/CoreAudio.cc
        src/drivers/CoreAudio.h
        src/drivers/MidiDriver.h
        src/drivers/OSSAudioDriver.cc
        src/drivers/OSSAudioDriver.h
        src/drivers/OSSMidiDriver.cc
        src/drivers/OSSMidiDriver.h
        src/GUI/bitmap_button.c
        src/GUI/bitmap_button.h
        src/GUI/bitmap_knob.c
        src/GUI/bitmap_knob.h
        src/GUI/bitmap_popup.c
        src/GUI/bitmap_popup.h
        src/GUI/ConfigDialog.cpp
        src/GUI/ConfigDialog.h
        src/GUI/controllers.h
        src/GUI/editor_menus.cpp
        src/GUI/editor_menus.h
        src/GUI/editor_pane.c
        src/GUI/editor_pane.h
        src/GUI/gui_main.cc
        src/GUI/gui_main.h
        src/GUI/MainMenu.cpp
        src/GUI/MainMenu.h
        src/GUI/MainWindow.cpp
        src/GUI/MainWindow.h
        src/GUI/MIDILearnDialog.cpp
        src/GUI/MIDILearnDialog.h
        src/GUI/PresetControllerView.cc
        src/GUI/PresetControllerView.h
        src/AudioOutput.cc
        src/AudioOutput.h
        src/JackOutput.cc
        src/JackOutput.h
        src/lash.c
        src/lash.h
        src/main.cc
        src/main.h
        )

target_link_libraries(amsynth
        pthread
        amsynth_core
        ${ALSA_LIBRARIES}
        ${JACK_LIBRARIES}
        ${GTK_LIBRARIES}
        ${LASH_LIBRARIES}
        ${SNDFILE_LIBRARIES}
        )

#
#
#

add_library(amsynth_dssi MODULE src/amsynth_dssi.cpp src/amsynth_dssi.h)
set_target_properties(amsynth_dssi PROPERTIES PREFIX "")
target_link_libraries(amsynth_dssi amsynth_core)

add_executable(amsynth_dssi_gtk
        src/amsynth_dssi_gtk.cpp
        src/GUI/bitmap_button.c
        src/GUI/bitmap_button.h
        src/GUI/bitmap_knob.c
        src/GUI/bitmap_knob.h
        src/GUI/bitmap_popup.c
        src/GUI/bitmap_popup.h
        src/GUI/controllers.h
        src/GUI/editor_menus.cpp
        src/GUI/editor_menus.h
        src/GUI/editor_pane.c
        src/GUI/editor_pane.h
        src/GUI/gui_main.cc
        src/GUI/gui_main.h
        )

target_link_libraries(amsynth_dssi_gtk
        amsynth_core
        ${GTK_LIBRARIES}
        ${LIBLO_LIBRARIES}
        )

#
#
#

add_library(amsynth_lv2 MODULE src/amsynth_lv2.cpp)
set_target_properties(amsynth_lv2 PROPERTIES PREFIX "")
target_link_libraries(amsynth_lv2 amsynth_core)


add_library(amsynth_lv2_gtk MODULE
        src/GUI/bitmap_button.c
        src/GUI/bitmap_button.h
        src/GUI/bitmap_knob.c
        src/GUI/bitmap_knob.h
        src/GUI/bitmap_popup.c
        src/GUI/bitmap_popup.h
        src/GUI/controllers.h
        src/GUI/editor_menus.cpp
        src/GUI/editor_menus.h
        src/GUI/editor_pane.c
        src/GUI/editor_pane.h
        src/GUI/gui_main.cc
        src/GUI/gui_main.h
        src/amsynth_lv2_ui_gtk.cpp
        )
set_target_properties(amsynth_lv2_gtk PROPERTIES PREFIX "")
target_link_libraries(amsynth_lv2_gtk amsynth_core ${GTK_LIBRARIES})

add_dependencies(amsynth_lv2 amsynth_lv2_gtk)

add_library(amsynth_vst MODULE
        src/amsynth_vst.cpp
        src/GUI/bitmap_button.c
        src/GUI/bitmap_button.h
        src/GUI/bitmap_knob.c
        src/GUI/bitmap_knob.h
        src/GUI/bitmap_popup.c
        src/GUI/bitmap_popup.h
        src/GUI/controllers.h
        src/GUI/editor_menus.cpp
        src/GUI/editor_menus.h
        src/GUI/editor_pane.c
        src/GUI/editor_pane.h
        src/GUI/gui_main.cc
        src/GUI/gui_main.h
        )
target_compile_options(amsynth_vst PUBLIC -DWITH_GUI)
set_target_properties(amsynth_vst PROPERTIES PREFIX "")
target_link_libraries(amsynth_vst amsynth_core ${GTK_LIBRARIES})


add_executable(amsynth_tests src/tests.cpp)
target_link_libraries(amsynth_tests amsynth_core)
