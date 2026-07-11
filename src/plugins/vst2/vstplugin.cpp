/*
 *  vstplugin.cpp
 *
 *  Copyright (c) 2014 Nick Dowell
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

#define JUCE_GUI_BASICS_INCLUDE_XHEADERS 1

#include "ardour/vestige.h"
#include "core/gui/MainComponent.h"
#include "core/gui/JuceIntegration.h"
#include "core/synth/PresetController.h"
#include "core/synth/Synthesizer.h"

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <memory>

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
#define kVstSysexType 6

// from https://github.com/DISTRHO/DPF/commit/fa54750583225c2e6aa5cda9baf9c18eb722daee
#define effEditKeyDown          59
#define effEditKeyUp            60

struct VstSysexEvent
{
	int type;
	int byteSize;
	int deltaSamples;
	int flags;
	int length;
	void *reserved1;
	void *data;
	void *reserved2;
};

struct Editor : public juce::Component
{
	Editor(PresetController *presetController, double scaleFactor)
	: main(presetController)
	{
		main.setTransform(juce::AffineTransform::scale(scaleFactor));
		setSize(main.getWidth() * scaleFactor, main.getHeight() * scaleFactor);
		addAndMakeVisible(main);
		setOpaque(true); // CreateWindowEx will fail if not opaque
	}

	void getRect(short **outPtr)
	{
		rect[2] = getHeight();
		rect[3] = getWidth();
		*outPtr = rect;
	}

	void open(void *ptr)
	{
		addToDesktop(juce::ComponentPeer::windowIgnoresKeyPresses, ptr);
		setVisible(true);
	}

	MainComponent main;
	short rect[4] {};
};

struct Plugin final : public Parameter::Observer
{
	Plugin(AEffect *effect, audioMasterCallback master)
	: effect(effect)
	{
		audioMaster = master;
		synthesizer = new Synthesizer;
		synthesizer->_presetController->getCurrentPreset().addObserver(this, false);
	}

	~Plugin()
	{ 
		delete synthesizer;
	}

	Editor *getEditor()
	{
		if (!editor) {
			editor = std::make_unique<Editor>(synthesizer->getPresetController(), juceIntegration.getPluginScaleFactor());
			for (const auto &it : synthesizer->getProperties()) {
				editor->main.propertyChanged(it.first.c_str(), it.second.c_str());
			}
			editor->main.sendProperty = [synthesizer = synthesizer] (const char *name, const char *value) {
				synthesizer->setProperty(name, value);
			};
		}
		return editor.get();
	}

	void parameterDidChange(const Parameter &parameter) override
	{
		if (audioMaster)
			audioMaster(effect, audioMasterAutomate, parameter.getId(), 0, nullptr, parameter.getNormalisedValue());
	}

	void parameterBeginEdit(const Parameter &parameter) override
	{
		if (audioMaster)
			audioMaster(effect, audioMasterBeginEdit, parameter.getId(), 0, nullptr, 0);
	}

	void parameterEndEdit(const Parameter &parameter) override
	{
		if (audioMaster)
			audioMaster(effect, audioMasterEndEdit, parameter.getId(), 0, nullptr, 0);
	}

	AEffect *effect;
	audioMasterCallback audioMaster;
	Synthesizer *synthesizer;
	MidiInputAdaptor midiInput;
	std::string chunk;
	JuceIntegration juceIntegration;
	std::unique_ptr<Editor> editor;
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

		case effSetProgram:
		case effGetProgram:
		case effGetProgramName:
			return 0;

		case effGetParamLabel:
			return 0;

		case effGetParamDisplay:
			parameter_get_display(index, plugin->synthesizer->_presetController->getCurrentPreset().getParameter(index).getValue(), (char *)ptr, 32);
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

		case effEditGetRect:
			plugin->getEditor()->getRect((short **)ptr);
			return 1;

		case effEditOpen:
			plugin->getEditor()->open(ptr);
			return 1;

		case effEditClose:
			if (plugin->editor) {
				plugin->editor->removeFromDesktop();
				plugin->editor.reset();
			}
			return 0;

		case effEditIdle:
			plugin->juceIntegration.idle();
			return 0;

#if JUCE_WINDOWS // Some hosts (e.g. Ableton Live) intercept all WM_CHAR messages etc.
		case effEditKeyDown:
			if (plugin->editor) {
				juce::ComponentPeer *peer = plugin->editor->getPeer();
				if (peer) {
					peer->handleKeyUpOrDown(true);
					if (peer->handleKeyPress(index, index))
						return 1;
				}
			}
			return 0;

		case effEditKeyUp:
			if (plugin->editor) {
				juce::ComponentPeer *peer = plugin->editor->getPeer();
				if (peer && peer->handleKeyUpOrDown(false))
					return 1;
			}
			return 0;
#endif

		case effGetChunk:
			plugin->chunk = plugin->synthesizer->getState();
			*(const char **)ptr = plugin->chunk.data();
			return plugin->chunk.size();

		case effSetChunk:
			plugin->synthesizer->setState(std::string((const char *)ptr, val));
			return 0;

		case effProcessEvents: {
			VstEvents *events = (VstEvents *)ptr;

			plugin->midiInput.clear();

			for (int32_t i=0; i<events->numEvents; i++) {
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-align"
#endif
				auto event = (const VstMidiEvent *)events->events[i];
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif
				if (event->type == kVstSysexType && event->byteSize == sizeof(VstSysexEvent)) {
					VstSysexEvent *sysex = (VstSysexEvent *)event;
					plugin->midiInput.append(event->deltaSamples, sysex->data, sysex->length);
					continue;
				}

				if (event->type != kVstMidiType) {
					continue;
				}

				static int lengths[8] = {3, 3, 3, 3, 2, 2, 3, 0};
				int pos = ((event->midiData[0] & 0xF0) - 0x80) >> 4;
				int msgLength = lengths[pos];
				plugin->midiInput.append(event->deltaSamples, (void *)event->midiData, msgLength);
			}
			
			return 1;
		}

		case effCanBeAutomated:
			return 1;

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
			return 0;

		case effGetTailSize:
		case effIdle:
			return 0;

		case effGetParameterProperties: {
			Parameter &parameter = plugin->synthesizer->_presetController->getCurrentPreset().getParameter(index);
			_VstParameterProperties *properties = (_VstParameterProperties *)ptr;
			memset(properties, 0, sizeof(*properties));
			if (parameter.getSteps() == 1)
				properties->flags |= kVstParameterIsSwitch;
			return 1;
		}

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
			return 0;
	}
}

static void process(AEffect *effect, float **inputs, float **outputs, int numSampleFrames)
{
	(void)inputs;
	Plugin *plugin = (Plugin *)effect->ptr3;
	std::vector<amsynth_midi_cc_t> midi_out;
	plugin->synthesizer->process(numSampleFrames, plugin->midiInput.events, midi_out, outputs[0], outputs[1]);
	plugin->midiInput.clear();
}

static void processReplacing(AEffect *effect, float **inputs, float **outputs, int numSampleFrames)
{
	(void)inputs;
	Plugin *plugin = (Plugin *)effect->ptr3;
	std::vector<amsynth_midi_cc_t> midi_out;
	plugin->synthesizer->process(numSampleFrames, plugin->midiInput.events, midi_out, outputs[0], outputs[1]);
	plugin->midiInput.clear();
}

static void setParameter(AEffect *effect, int i, float f)
{
	Plugin *plugin = (Plugin *)effect->ptr3;
	plugin->synthesizer->_presetController->getCurrentPreset().getParameter(i).setNormalisedValue(f, plugin);
}

static float getParameter(AEffect *effect, int i)
{
	Plugin *plugin = (Plugin *)effect->ptr3;
	return plugin->synthesizer->getNormalizedParameterValue((Param)i);
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
	AEffect *effect = (AEffect *)calloc(1, sizeof(AEffect));
	effect->magic = kEffectMagic;
	effect->dispatcher = dispatcher;
	effect->process = process;
	effect->setParameter = setParameter;
	effect->getParameter = getParameter;
	effect->numPrograms = 0;
	effect->numParams = kAmsynthParameterCount;
	effect->numInputs = 0;
	effect->numOutputs = 2;
	effect->flags = effFlagsCanReplacing | effFlagsIsSynth | effFlagsProgramChunks | effFlagsHasEditor;
	effect->ptr3 = new Plugin(effect, audioMaster);
	effect->uniqueID = CCONST('a', 'm', 's', 'y');
	effect->processReplacing = processReplacing;
	return effect;
}

AEffect * MAIN(audioMasterCallback audioMaster)
{
	return VSTPluginMain (audioMaster);
}
