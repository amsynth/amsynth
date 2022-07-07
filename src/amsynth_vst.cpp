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
#include "GUI/editor_pane.h"
#include <gdk/gdkx.h>
#if __x86_64__
#include <sys/mman.h>
#include <sys/user.h>
#endif
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
		if (g_thread_self() == mainThread) {
			updateEditorParameter(paramID, paramValue);
		} else {
			g_async_queue_push(parameterUpdateQueue, new ParameterUpdate(paramID, paramValue));
			g_idle_add(Plugin::idleCallback, this);
		}
	}

	static gboolean idleCallback(gpointer data)
	{
		Plugin *plugin = (Plugin *)data;
		ParameterUpdate *update;
		while ((update = (ParameterUpdate *)g_async_queue_try_pop(plugin->parameterUpdateQueue))) {
			plugin->updateEditorParameter(update->first, update->second);
			delete update;
		}
		return G_SOURCE_REMOVE;
	}

	void updateEditorParameter(int parameter, float paramValue)
	{
		if (0 <= parameter && parameter < kAmsynthParameterCount) {
			gdouble value = synthesizer->getParameterValue((Param)parameter);
			ignoreAdjustmentValueChanges = true;
			gtk_adjustment_set_value(adjustments[parameter], value);
			ignoreAdjustmentValueChanges = false;
		}
	}

	GdkWindow *gdkParentWindow = nullptr;
	GtkWidget *gtkWindow = nullptr;
	GtkWidget *editorWidget = nullptr;
	GThread *mainThread = nullptr;
	GAsyncQueue *parameterUpdateQueue = nullptr;
	bool ignoreAdjustmentValueChanges = false;
	GtkAdjustment *adjustments[kAmsynthParameterCount] = {0};
#endif
};

#ifdef WITH_GUI

static void on_adjustment_value_changed(GtkAdjustment *adjustment, AEffect *effect)
{
	Plugin *plugin = (Plugin *)effect->ptr3;
	if (plugin->ignoreAdjustmentValueChanges) {
		return;
	}

	static Preset dummyPreset;

	for (int i = 0; i < kAmsynthParameterCount; i++) {
		if (adjustment == plugin->adjustments[i]) {
			float value = gtk_adjustment_get_value(adjustment);
			Parameter &param = dummyPreset.getParameter(i);
			param.setValue(value);
			plugin->synthesizer->setParameterValue((Param)i, value);
			if (plugin->audioMaster && !strstr(hostProductString, "Qtractor")) {
				plugin->audioMaster(effect, audioMasterAutomate, i, 0, nullptr, param.getNormalisedValue());
			}
		}
	}
}

void modal_midi_learn(Param param_index) {}

static void XEventProc(XEvent *xevent)
{
	xevent->xany.display = GDK_DISPLAY_XDISPLAY(gdk_display_get_default());
	XPutBackEvent(xevent->xany.display, xevent);
	gtk_main_iteration();
}

static void setEventProc(Display *display, Window window)
{
#if __x86_64__
	//
	// JUCE calls XGetWindowProperty with long_length = 1 which means it only fetches the lower 32 bits of the address.
	// Therefore we need to ensure we return an address in the lower 32-bits of address space.
	//

	// based on mach_override
	static const unsigned char kJumpInstructions[] = {
			0xFF, 0x25, 0x00, 0x00, 0x00, 0x00,
	};

	static char *ptr;
	if (!ptr) {
		ptr = (char *)mmap(nullptr,
						   PAGE_SIZE,
						   PROT_READ | PROT_WRITE | PROT_EXEC,
						   MAP_ANONYMOUS | MAP_PRIVATE | MAP_32BIT,
						   0, 0);
		if (ptr == MAP_FAILED) {
			perror("mmap");
			ptr = nullptr;
			return;
		} else {
			memcpy(ptr, kJumpInstructions, sizeof(kJumpInstructions));
			memcpy(ptr + sizeof(kJumpInstructions), (const void *)&XEventProc, sizeof(&XEventProc));
		}
	}

	long temp[2] = {(long)ptr, 0};
	Atom atom = XInternAtom(display, "_XEventProc", false);
	XChangeProperty(display, window, atom, atom, 32, PropModeReplace, (unsigned char *)temp, 2);
#else
	long temp[1] = {(long)(void *)(&XEventProc)};
	Atom atom = XInternAtom(display, "_XEventProc", false);
	XChangeProperty(display, window, atom, atom, 32, PropModeReplace, (unsigned char *)temp, 1);
#endif
}

#if defined(DEBUG) && DEBUG
static void gdk_event_handler(GdkEvent *event, gpointer	data)
{
	static const char *names[] = {
			"GDK_DELETE",
			"GDK_DESTROY",
			"GDK_EXPOSE",
			"GDK_MOTION_NOTIFY",
			"GDK_BUTTON_PRESS",
			"GDK_2BUTTON_PRESS",
			"GDK_3BUTTON_PRESS",
			"GDK_BUTTON_RELEASE",
			"GDK_KEY_PRESS",
			"GDK_KEY_RELEASE",
			"GDK_ENTER_NOTIFY",
			"GDK_LEAVE_NOTIFY",
			"GDK_FOCUS_CHANGE",
			"GDK_CONFIGURE",
			"GDK_MAP",
			"GDK_UNMAP",
			"GDK_PROPERTY_NOTIFY",
			"GDK_SELECTION_CLEAR",
			"GDK_SELECTION_REQUEST",
			"GDK_SELECTION_NOTIFY",
			"GDK_PROXIMITY_IN",
			"GDK_PROXIMITY_OUT",
			"GDK_DRAG_ENTER",
			"GDK_DRAG_LEAVE",
			"GDK_DRAG_MOTION",
			"GDK_DRAG_STATUS",
			"GDK_DROP_START",
			"GDK_DROP_FINISHED",
			"GDK_CLIENT_EVENT",
			"GDK_VISIBILITY_NOTIFY",
			"GDK_NO_EXPOSE",
			"GDK_SCROLL",
			"GDK_WINDOW_STATE",
			"GDK_SETTING",
			"GDK_OWNER_CHANGE",
			"GDK_GRAB_BROKEN",
			"GDK_DAMAGE"
	};
	fprintf(stderr, "%22s window = %p send_event = %d\n",
			names[event->any.type], event->any.window, event->any.send_event);
	gtk_main_do_event(event);
}
#endif // DEBUG

static void init_gtk()
{
	static bool initialized = false;
	if (!initialized) {
#if defined(DEBUG) && DEBUG
		int argc = 2;
		char arg1[32] = "/dev/null";
		char arg2[32] = "--g-fatal-warnings";
		char *args[] = { arg1, arg2 };
		char **argv = args;
		gtk_init(&argc, &argv);
		gdk_event_handler_set(&gdk_event_handler, NULL, NULL);
#else
		gtk_init(nullptr, nullptr);
#endif
		initialized = true;
	}
}

#endif // WITH_GUI

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
			init_gtk();
			int scale = default_scaling_factor();
			rect.bottom = 400 * scale;
			rect.right = 600 * scale;
			*(ERect **)ptr = &rect;
			return 1;
		}
		case effEditOpen: {
			init_gtk();

			if (!plugin->editorWidget) {
				plugin->mainThread = g_thread_self();
				plugin->parameterUpdateQueue = g_async_queue_new();
				for (int i = 0; i < kAmsynthParameterCount; i++) {
					gdouble lower = 0, upper = 0, step_increment = 0;
					get_parameter_properties(i, &lower, &upper, nullptr, &step_increment);
					gdouble value = plugin->synthesizer->getParameterValue((Param)i);
					plugin->adjustments[i] = (GtkAdjustment *)gtk_adjustment_new(value, lower, upper, step_increment, 0, 0);
					g_object_ref_sink(plugin->adjustments[i]); // assumes ownership of the floating reference
					g_signal_connect(plugin->adjustments[i], "value-changed", G_CALLBACK(on_adjustment_value_changed), effect);
					plugin->synthesizer->getPresetController()->getCurrentPreset().getParameter((Param)i).addUpdateListener(plugin);
				}
				plugin->editorWidget = editor_pane_new(plugin->synthesizer, plugin->adjustments, TRUE, DEFAULT_SCALING);
				g_object_ref_sink(plugin->editorWidget);
			}

			plugin->gtkWindow = gtk_window_new(GTK_WINDOW_POPUP);

			// don't show the widget yet, to avoid visible moving of the window
			gtk_widget_realize(plugin->gtkWindow); g_assert(gtk_widget_get_realized(plugin->gtkWindow));

			// on some hosts (e.g. energyXT) creating the gdk window can fail unless we call gdk_display_sync
			gdk_display_sync(gdk_display_get_default());

			plugin->gdkParentWindow = gdk_x11_window_foreign_new_for_display(gdk_display_get_default(), (GdkNativeWindow)(uintptr_t)ptr); //gdk_window_foreign_new((GdkNativeWindow)(uintptr_t)ptr);
			g_assert(plugin->gdkParentWindow);

			// use gdk_window_reparent instead of XReparentWindow to avoid "GdkWindow unexpectedly destroyed" warnings
			gdk_window_reparent(gtk_widget_get_window(plugin->gtkWindow), plugin->gdkParentWindow, 0, 0);

			Display *xdisplay = GDK_DISPLAY_XDISPLAY(gdk_display_get_default());
			Window xwindow = GDK_WINDOW_XWINDOW(gtk_widget_get_window(plugin->gtkWindow));
			setEventProc(xdisplay, xwindow);

			gtk_container_add(GTK_CONTAINER(plugin->gtkWindow), plugin->editorWidget);
			gtk_widget_show_all(plugin->gtkWindow);

			gdk_display_sync(gdk_display_get_default());

			return 1;
		}
		case effEditClose: {
			for (int i = 0; i < kAmsynthParameterCount; i++) {
				plugin->synthesizer->getPresetController()->getCurrentPreset().getParameter((Param)i).removeUpdateListener(plugin);
			}

			if (plugin->gtkWindow) {
				if (gtk_widget_get_window(plugin->gtkWindow)) {
					gdk_window_hide(gtk_widget_get_window(plugin->gtkWindow));
				}
				gtk_widget_destroy(plugin->gtkWindow);
				plugin->gtkWindow = nullptr;
			}

			for (int i = 0; i < kAmsynthParameterCount; i++) {
				plugin->adjustments[i] = nullptr;
			}

			g_async_queue_unref(plugin->parameterUpdateQueue);
			plugin->parameterUpdateQueue = nullptr;
			plugin->gdkParentWindow = nullptr;
			plugin->editorWidget = nullptr;

			gdk_display_sync(gdk_display_get_default());

			return 0;
		}
		case effEditIdle: {
			while (gtk_events_pending()) {
				gtk_main_iteration();
			}
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

#ifdef _WIN32
extern "C" __declspec(dllexport)
AEffect * VSTPluginMain(audioMasterCallback audioMaster)
#else
extern "C" __attribute__ ((visibility("default")))
AEffect * VSTPluginMain(audioMasterCallback audioMaster);
AEffect * VSTPluginMain(audioMasterCallback audioMaster)
#endif
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
	if (strcmp("REAPER", hostProductString) == 0) {
		// amsynth's GTK GUI doesn't work in REAPER :-[
	} else {
		effect->flags |= effFlagsHasEditor;
	}
#endif
	// Do no use the ->user pointer because ardour clobbers it
	effect->ptr3 = new Plugin(audioMaster);
	effect->uniqueID = CCONST('a', 'm', 's', 'y');
	effect->processReplacing = processReplacing;
	return effect;
}

#ifdef _WIN32

__declspec(dllexport)
extern "C" AEffect * MAIN(audioMasterCallback audioMaster)
{
	return VSTPluginMain (audioMaster);
}

#else

// this is required because GCC throws an error if we declare a non-standard function named 'main'
extern "C" __attribute__ ((visibility("default"))) AEffect * main_plugin(audioMasterCallback audioMaster) asm ("main");

extern "C" __attribute__ ((visibility("default"))) AEffect * main_plugin(audioMasterCallback audioMaster)
{
	return VSTPluginMain (audioMaster);
}

#endif
