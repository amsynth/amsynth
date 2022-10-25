/*
 *  amsynth_vst.cpp
 *
 *  Copyright (c) 2008-2022 Nick Dowell
 *
 *  This file is part of amsynth.
 *
 *  amsynth is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  amsynth is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with amsynth.  If not, see <http://www.gnu.org/licenses/>.
 */

#if HAVE_CONFIG_H
#include "config.h"
#endif

#include "midi.h"
#include "Preset.h"
#include "PresetController.h"
#include "Synthesizer.h"

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <memory>

#include <vestige/aeffectx.h>

// from http://www.asseca.org/vst-24-specs/index.html
#define effGetParamLabel        6
#define effGetParamDisplay      7
#define effGetChunk             23
#define effSetChunk             24
#define effCanBeAutomated       26
#define effGetOutputProperties  34
#define effGetTailSize          52
#define effGetMidiKeyName       66
#define effBeginLoadBank        75
#define effFlagsProgramChunks   (1 << 5)

#ifdef WITH_GUI
#define JUCE_GUI_BASICS_INCLUDE_XHEADERS 1
#include <juce_core/juce_core.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_events/juce_events.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_gui_basics/juce_gui_basics.h>
// Must be included after juce_core
#include <juce_audio_plugin_client/utility/juce_LinuxMessageThread.h>
#endif

struct ERect
{
	short top;
	short left;
	short bottom;
	short right;
};

#define MIDI_BUFFER_SIZE 4096

static char hostProductString[64] = "";

#if defined(DEBUG) && DEBUG
static FILE *logFile;
#endif

constexpr size_t kPresetsPerBank = sizeof(BankInfo::presets) / sizeof(BankInfo::presets[0]);

#ifdef WITH_GUI

extern "C" void modal_midi_learn(Param param_index) {}

struct Editor : public juce::Component
{
	Editor()
	{
		setSize(600, 400);
		slider.setOpaque(true);
		slider.setSize(100, 100);
		addAndMakeVisible(slider);
		setOpaque(true);
	}

	void attachToHost(void *hostWindow)
	{
		setVisible(false);
		addToDesktop(0, hostWindow);
		auto display = juce::XWindowSystem::getInstance()->getDisplay();
		juce::X11Symbols::getInstance()->xReparentWindow(display,
			(::Window)getWindowHandle(),
			(::Window)hostWindow,
			0, 0);
		juce::X11Symbols::getInstance()->xFlush(display);
		setVisible(true);
	}

	juce::Slider slider{
		juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag,
		juce::Slider::TextEntryBoxPosition::TextBoxBelow};

	juce::SharedResourcePointer<juce::HostDrivenEventLoop> hostEventLoop;
};

#endif // WITH_GUI

struct Plugin : public UpdateListener
{
	Plugin(audioMasterCallback master)
	{
		audioMaster = master;
		synthesizer = new Synthesizer;
		midiBuffer = (unsigned char *)malloc(MIDI_BUFFER_SIZE);
	}

	~Plugin()
	{ 
		delete synthesizer;
		free(midiBuffer);
	}

	audioMasterCallback audioMaster;
	Synthesizer *synthesizer;
	unsigned char *midiBuffer;
	std::vector<amsynth_midi_event_t> midiEvents;
	int programNumber = 0;
	std::string presetName;

#ifdef WITH_GUI
	typedef std::pair<Param, float> ParameterUpdate;

	void UpdateParameter(Param paramID, float paramValue) override
	{
		// TODO: Update Editor
	}

	juce::ScopedJuceInitialiser_GUI libraryInitialiser;
	juce::SharedResourcePointer<juce::MessageThread> messageThread;
	std::unique_ptr<Editor> editor;
#endif
};

static intptr_t dispatcher(AEffect *effect, int opcode, int index, intptr_t val, void *ptr, float f)
{
	Plugin *plugin = (Plugin *)effect->ptr3;

	switch (opcode) {
		case effOpen:
			return 0;

		case effClose:
			delete plugin;
			memset(effect, 0, sizeof(AEffect));
			free(effect);
			return 0;

		case effSetProgram: {
			auto &bank = PresetController::getPresetBanks().at(val / kPresetsPerBank);
			auto &preset = bank.presets[val % kPresetsPerBank];
			plugin->presetName = preset.getName();
			plugin->programNumber = val;
			plugin->synthesizer->_presetController->setCurrentPreset(preset);
			return 1;
		}

		case effGetProgram:
			return plugin->programNumber;

		case effGetProgramName:
			strncpy((char *)ptr, plugin->presetName.c_str(), 24);
			return 1;

		case effGetParamLabel:
			plugin->synthesizer->getParameterLabel((Param)index, (char *)ptr, 32);
			return 0;

		case effGetParamDisplay:
			plugin->synthesizer->getParameterDisplay((Param)index, (char *)ptr, 32);
			return 0;

		case effGetParamName:
			plugin->synthesizer->getParameterName((Param)index, (char *)ptr, 32);
			return 0;

		case effSetSampleRate:
			plugin->synthesizer->setSampleRate(f);
			return 0;

		case effSetBlockSize:
		case effMainsChanged:
			return 0;

#ifdef WITH_GUI
		case effEditGetRect: {
			static ERect rect;
			auto scale = 1;
			rect.bottom = 400 * scale;
			rect.right = 600 * scale;
			*(ERect **)ptr = &rect;
			return 1;
		}

		case effEditOpen: {
			juce::SharedResourcePointer<juce::HostDrivenEventLoop> hostDrivenEventLoop;
			plugin->editor = std::make_unique<Editor>();
			plugin->editor->attachToHost(ptr);
			return 1;
		}

		case effEditClose: {
			juce::SharedResourcePointer<juce::HostDrivenEventLoop> hostDrivenEventLoop;
			plugin->editor.reset();
			return 0;
		}

		case effEditIdle: {
			juce::SharedResourcePointer<juce::HostDrivenEventLoop> hostDrivenEventLoop;
			hostDrivenEventLoop->processPendingEvents();
			return 0;
		}
#endif

		case effGetChunk:
			return plugin->synthesizer->saveState((char **)ptr);

		case effSetChunk:
			plugin->synthesizer->loadState((char *)ptr);
			return 0;

		case effProcessEvents: {
			VstEvents *events = (VstEvents *)ptr;

			plugin->midiEvents.clear();
			memset(plugin->midiBuffer, 0, MIDI_BUFFER_SIZE);
			size_t bytesCopied = 0;
			
			for (int32_t i=0; i<events->numEvents; i++) {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
				auto event = (const VstMidiEvent *)events->events[i];
#pragma GCC diagnostic pop
				if (event->type != kVstMidiType) {
					continue;
				}

				unsigned char *msgData = (unsigned char *)event->midiData;

				int msgLength = 0;
				switch (msgData[0] & 0xF0) {
				case MIDI_STATUS_NOTE_OFF:
				case MIDI_STATUS_NOTE_ON:
				case MIDI_STATUS_NOTE_PRESSURE:
				case MIDI_STATUS_CONTROLLER:
				case MIDI_STATUS_PITCH_WHEEL:
					msgLength = 3;
					break;
				case MIDI_STATUS_PROGRAM_CHANGE:
				case MIDI_STATUS_CHANNEL_PRESSURE:
					msgLength = 2;
					break;
				case 0xF0: // System message
					continue; // Ignore
				default:
					fprintf(stderr, "amsynth: bad status byte: %02x\n", msgData[0]);
					continue; // Ignore
				}

				if (bytesCopied + msgLength > MIDI_BUFFER_SIZE) {
					fprintf(stderr, "amsynth: midi buffer overflow\n");
					break;
				}

				amsynth_midi_event_t midi_event;
				midi_event.offset_frames = event->deltaFrames;
				midi_event.length = msgLength;
				midi_event.buffer = (unsigned char *)memcpy(plugin->midiBuffer + bytesCopied, msgData, msgLength);
				plugin->midiEvents.push_back(midi_event);
				bytesCopied += msgLength;
			}
			
			return 1;
		}

		case effCanBeAutomated:
		case effGetOutputProperties:
			return 0;

		case effGetPlugCategory:
			return kPlugCategSynth;
		case effGetEffectName:
			strcpy((char *)ptr, "amsynth");
			return 1;
		case effGetVendorString:
			strcpy((char *)ptr, "Nick Dowell");
			return 1;
		case effGetProductString:
			strcpy((char *)ptr, "amsynth");
			return 1;
		case effGetVendorVersion:
			return 0;

		case effCanDo:
			if (strcmp("receiveVstMidiEvent", (char *)ptr) == 0 ||
				false) return 1;
			if (strcmp("midiKeyBasedInstrumentControl", (char *)ptr) == 0 ||
				strcmp("midiSingleNoteTuningChange", (char *)ptr) == 0 ||
				strcmp("receiveVstSysexEvent", (char *)ptr) == 0 ||
				strcmp("sendVstMidiEvent", (char *)ptr) == 0 ||
				false) return 0;
#if defined(DEBUG) && DEBUG
			fprintf(logFile, "[amsynth_vst] unhandled canDo: %s\n", (char *)ptr);
			fflush(logFile);
#endif
			return 0;

		case effGetTailSize:
		case effIdle:
		case effGetParameterProperties:
			return 0;

		case effGetVstVersion:
			return 2400;

		case effGetMidiKeyName:
		case effStartProcess:
		case effStopProcess:
		case effBeginSetProgram:
		case effEndSetProgram:
		case effBeginLoadBank:
			return 0;

		default:
#if defined(DEBUG) && DEBUG
			fprintf(logFile, "[amsynth_vst] unhandled VST opcode: %d\n", opcode);
			fflush(logFile);
#endif
			return 0;
	}
}

static void process(AEffect *effect, float **inputs, float **outputs, int numSampleFrames)
{
	Plugin *plugin = (Plugin *)effect->ptr3;
	std::vector<amsynth_midi_cc_t> midi_out;
	plugin->synthesizer->process(numSampleFrames, plugin->midiEvents, midi_out, outputs[0], outputs[1]);
	plugin->midiEvents.clear();
}

static void processReplacing(AEffect *effect, float **inputs, float **outputs, int numSampleFrames)
{
	Plugin *plugin = (Plugin *)effect->ptr3;
	std::vector<amsynth_midi_cc_t> midi_out;
	plugin->synthesizer->process(numSampleFrames, plugin->midiEvents, midi_out, outputs[0], outputs[1]);
	plugin->midiEvents.clear();
}

static void setParameter(AEffect *effect, int i, float f)
{
	Plugin *plugin = (Plugin *)effect->ptr3;
	plugin->synthesizer->setNormalizedParameterValue((Param) i, f);
}

static float getParameter(AEffect *effect, int i)
{
	Plugin *plugin = (Plugin *)effect->ptr3;
	return plugin->synthesizer->getNormalizedParameterValue((Param) i);
}

static int getNumPrograms()
{
	return PresetController::getPresetBanks().size() * kPresetsPerBank;
}

extern "C" {
#ifdef _WIN32
__declspec(dllexport) AEffect * MAIN(audioMasterCallback);
__declspec(dllexport) AEffect * VSTPluginMain(audioMasterCallback);
#else // https://gcc.gnu.org/onlinedocs/gcc/Asm-Labels.html
__attribute__ ((visibility("default"))) AEffect * MAIN(audioMasterCallback) asm ("main");
__attribute__ ((visibility("default"))) AEffect * VSTPluginMain(audioMasterCallback);
#endif
}

AEffect * VSTPluginMain(audioMasterCallback audioMaster)
{
#if defined(DEBUG) && DEBUG
	if (!logFile) {
		logFile = fopen("/tmp/amsynth.log", "a");
	}
#endif
	if (audioMaster) {
		audioMaster(nullptr, audioMasterGetProductString, 0, 0, hostProductString, 0.0f);
	}
	AEffect *effect = (AEffect *)calloc(1, sizeof(AEffect));
	effect->magic = kEffectMagic;
	effect->dispatcher = dispatcher;
	effect->process = process;
	effect->setParameter = setParameter;
	effect->getParameter = getParameter;
	effect->numPrograms = getNumPrograms();
	effect->numParams = kAmsynthParameterCount;
	effect->numInputs = 0;
	effect->numOutputs = 2;
	effect->flags = effFlagsCanReplacing | effFlagsIsSynth | effFlagsProgramChunks;
#ifdef WITH_GUI
	effect->flags |= effFlagsHasEditor;
#endif
	// Do no use the ->user pointer because ardour clobbers it
	effect->ptr3 = new Plugin(audioMaster);
	effect->uniqueID = CCONST('a', 'm', 's', 'y');
	effect->processReplacing = processReplacing;
	return effect;
}

AEffect * MAIN(audioMasterCallback audioMaster)
{
	return VSTPluginMain (audioMaster);
}
