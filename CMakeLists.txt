cmake_minimum_required(VERSION 3.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_VERBOSE_MAKEFILE true)

find_package(PkgConfig REQUIRED)

project(amsynth)

set(SOURCE_FILES
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
        src/GUI/bitmap_button.c
        src/GUI/bitmap_button.h
        src/GUI/bitmap_knob.c
        src/GUI/bitmap_knob.h
        src/GUI/bitmap_popup.c
        src/GUI/bitmap_popup.h
        src/GUI/ConfigDialog.cpp
        src/GUI/ConfigDialog.h
        src/GUI/controller_menu.cpp
        src/GUI/controller_menu.h
        src/GUI/controllers.h
        src/GUI/editor_pane.c
        src/GUI/editor_pane.h
        src/GUI/GUI.cc
        src/GUI/GUI.h
        src/GUI/gui_main.cc
        src/GUI/gui_main.h
        src/GUI/MIDILearnDialog.cpp
        src/GUI/MIDILearnDialog.h
        src/GUI/PresetControllerView.cc
        src/GUI/PresetControllerView.h
        src/GUI/presets_menu.cpp
        src/GUI/presets_menu.h
        src/GUI/Request.h
        src/VoiceBoard/ADSR.cc
        src/VoiceBoard/ADSR.h
        src/VoiceBoard/LowPassFilter.cc
        src/VoiceBoard/LowPassFilter.h
        src/VoiceBoard/Oscillator.cc
        src/VoiceBoard/Oscillator.h
        src/VoiceBoard/Synth--.h
        src/VoiceBoard/VoiceBoard.cc
        src/VoiceBoard/VoiceBoard.h
        src/amsynth_lv2.cpp
#        src/amsynth_vst.cpp
        src/AudioOutput.cc
        src/AudioOutput.h
        src/Configuration.cpp
        src/Configuration.h
        src/controls.h
        src/gettext.h
        src/JackOutput.cc
        src/JackOutput.h
        src/lash.c
        src/lash.h
        src/main.cc
        src/main.h
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

add_executable(amsynth ${SOURCE_FILES})

pkg_check_modules(ALSA REQUIRED alsa)
pkg_check_modules(JACK REQUIRED jack)
pkg_check_modules(GTKMM REQUIRED gtkmm-2.4)

include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${ALSA_INCLUDE_DIRS}
        ${JACK_INCLUDE_DIRS}
        ${GTKMM_INCLUDE_DIRS}
)

target_compile_options(
        amsynth
        PUBLIC
        -DVERSION="dev"
        -DPACKAGE="amsynth"
        -DPACKAGE_NAME="amsynth"
        -DPACKAGE_TARNAME="amsynth"
        -DPACKAGE_STRING="amsynth dev"
        -DGETTEXT_PACKAGE="amsynth"
        -DPACKAGE_URL="https://amsynth.github.io"
        -DPACKAGE_BUGREPORT="https://www.github.com/amsynth/amsynth/issues"
        -DPKGDATADIR="/home/nick/.local/share/amsynth"
        -DPACKAGE_LOCALEDIR="/home/nick/.local/share/locale"
        -DWITH_GUI
        -DWITH_ALSA
        -DWITH_JACK
        -DHAVE_JACK_MIDIPORT_H
        -DHAVE_JACK_SESSION_H
        ${ALSA_CFLAGS_OTHER}
        ${JACK_CFLAGS_OTHER}
        ${GTKMM_CFLAGS_OTHER}
)

target_link_libraries(
        amsynth
        pthread
        ${ALSA_LIBRARIES}
        ${JACK_LIBRARIES}
        ${GTKMM_LIBRARIES}
)
