/*
 *  amsynth_vst.cpp
 *
 *  Copyright (c) 2008-2015 Nick Dowell
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

#include "Preset.h"
#include "Synthesizer.h"

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <vestige/aeffectx.h>

#ifdef WITH_GUI
#include "GUI/editor_pane.h"
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#endif

struct ERect
{
	short top;
	short left;
	short bottom;
	short right;
};

static char hostProductString[64] = "";

struct Plugin
{
	Plugin(audioMasterCallback master)
	{
		audioMaster = master;
		synthesizer = new Synthesizer;
		midiBuffer = (unsigned char *)malloc(4096);
#ifdef WITH_GUI
		gdkParentWindow = 0;
		gtkWindow = 0;
		editorWidget = 0;
#endif
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
#ifdef WITH_GUI
	GdkWindow *gdkParentWindow;
	GtkWidget *gtkWindow;
	GtkWidget *editorWidget;
	GtkAdjustment *adjustments[kAmsynthParameterCount];
#endif
};

#ifdef WITH_GUI
static void on_adjustment_value_changed(GtkAdjustment *adjustment, AEffect *effect)
{
	Plugin *plugin = (Plugin *)effect->ptr3;

	static Preset dummyPreset;

	for (int i = 0; i < kAmsynthParameterCount; i++) {
		if (adjustment == plugin->adjustments[i]) {
			float value = gtk_adjustment_get_value(adjustment);
			Parameter &param = dummyPreset.getParameter(i);
			param.setValue(value);
			plugin->synthesizer->setParameterValue((Param)i, value);
			if (plugin->audioMaster && !strstr(hostProductString, "Qtractor")) {
				plugin->audioMaster(effect, audioMasterAutomate, i, 0, 0, param.GetNormalisedValue());
			}
		}
	}
}
#endif

void modal_midi_learn(int param_index) {}

static intptr_t dispatcher(AEffect *effect, int opcode, int index, intptr_t val, void *ptr, float f)
{
	Plugin *plugin = (Plugin *)effect->ptr3;

	switch (opcode) {
		case 6:
			plugin->synthesizer->getParameterLabel((Param)index, (char *)ptr, 32);
			return 0;
		case 7:
			plugin->synthesizer->getParameterDisplay((Param)index, (char *)ptr, 32);
			return 0;
		case effGetParamName:
			plugin->synthesizer->getParameterName((Param)index, (char *)ptr, 32);
			return 0;

		case effSetSampleRate:
			plugin->synthesizer->setSampleRate(f);
			return 0;
		case effSetBlockSize:
			return 0;
		case effMainsChanged:
			return 0;

#ifdef WITH_GUI
		case effEditGetRect: {
			static ERect rect = {0, 0, 400, 600};
			ERect **er = (ERect **)ptr;
			*er = &rect;
			return 1;
		}
		case effEditOpen: {
			static bool initialized = false;
			if (!initialized) {
				gtk_init(NULL, NULL);
				initialized = true;
			}

			if (!plugin->editorWidget) {
				for (int i = 0; i < kAmsynthParameterCount; i++) {
					gdouble value = 0, lower = 0, upper = 0, step_increment = 0;
					get_parameter_properties(i, &lower, &upper, &value, &step_increment);
					plugin->adjustments[i] = (GtkAdjustment *)gtk_adjustment_new(value, lower, upper, step_increment, 0, 0);
					g_object_ref_sink(plugin->adjustments[i]); // assumes ownership of the floating reference
					g_signal_connect(plugin->adjustments[i], "value-changed", G_CALLBACK(on_adjustment_value_changed), effect);
				}
				plugin->editorWidget = editor_pane_new(plugin->adjustments, TRUE);
				g_object_ref_sink(plugin->editorWidget);
			}

			plugin->gtkWindow = gtk_window_new(GTK_WINDOW_POPUP);

			// don't show the widget yet, to avoid visible moving of the window
			gtk_widget_realize(plugin->gtkWindow); g_assert(gtk_widget_get_realized(plugin->gtkWindow));

			// on some hosts (e.g. energyXT) creating the gdk window can fail unless we call gdk_display_sync
			gdk_display_sync(gdk_display_get_default());

			plugin->gdkParentWindow = gdk_window_foreign_new((GdkNativeWindow)(uintptr_t)ptr);
			g_assert(plugin->gdkParentWindow);

			// use gdk_window_reparent instead of XReparentWindow to avoid "GdkWindow unexpectedly destroyed" warnings
			gdk_window_reparent(gtk_widget_get_window(plugin->gtkWindow), plugin->gdkParentWindow, 0, 0);

			gtk_container_add(GTK_CONTAINER(plugin->gtkWindow), plugin->editorWidget);
			gtk_widget_show_all(plugin->gtkWindow);

			gdk_display_sync(gdk_display_get_default());

			return 1;
		}
		case effEditClose: {
			if (plugin->gtkWindow) {
				if (gtk_widget_get_window(plugin->gtkWindow)) {
					gdk_window_hide(gtk_widget_get_window(plugin->gtkWindow));
				}
				gtk_widget_destroy(plugin->gtkWindow);
				plugin->gtkWindow = 0;
			}

			for (int i = 0; i < kAmsynthParameterCount; i++) {
				plugin->adjustments[i] = 0;
			}

			plugin->gdkParentWindow = 0;
			plugin->editorWidget = 0;

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

		case effProcessEvents: {
			VstEvents *events = (VstEvents *)ptr;

			assert(plugin->midiEvents.empty());

			memset(plugin->midiBuffer, 0, 4096);
			unsigned char *buffer = plugin->midiBuffer;
			
			for (int32_t i=0; i<events->numEvents; i++) {
				VstMidiEvent *event = (VstMidiEvent *)events->events[i];
				if (event->type != kVstMidiType) {
					continue;
				}

				memcpy(buffer, event->midiData, 4);

				amsynth_midi_event_t midi_event;
				memset(&midi_event, 0, sizeof(midi_event));
				midi_event.offset_frames = event->deltaFrames;
				midi_event.buffer = buffer;
				midi_event.length = 4;
				plugin->midiEvents.push_back(midi_event);

				buffer += event->byteSize;

				assert(buffer < plugin->midiBuffer + 4096);
			}
			
			return 1;
		}
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
			if (strcmp("receiveVstMidiEvent", (char *)ptr) == 0)
				return 1;
			return 0;
		case effGetParameterProperties:
			return 0;
		case effGetVstVersion:
			return 2400;
		default:
			return 0;
	}
}

static void process(AEffect *effect, float **inputs, float **outputs, int numSampleFrames)
{
	Plugin *plugin = (Plugin *)effect->ptr3;
	plugin->synthesizer->process(numSampleFrames, plugin->midiEvents, outputs[0], outputs[1]);
	plugin->midiEvents.clear();
}

static void processReplacing(AEffect *effect, float **inputs, float **outputs, int numSampleFrames)
{
	Plugin *plugin = (Plugin *)effect->ptr3;
	plugin->synthesizer->process(numSampleFrames, plugin->midiEvents, outputs[0], outputs[1]);
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

extern "C" AEffect * VSTPluginMain(audioMasterCallback audioMaster)
{
	if (audioMaster) {
		audioMaster(NULL, audioMasterGetProductString, 0, 0, hostProductString, 0.0f);
	}
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
	effect->flags = effFlagsCanReplacing | effFlagsIsSynth;
#ifdef WITH_GUI
	effect->flags |= effFlagsHasEditor;
#endif
	// Do no use the ->user pointer because ardour clobbers it
	effect->ptr3 = new Plugin(audioMaster);
	effect->uniqueID = CCONST('a', 'm', 's', 'y');
	effect->processReplacing = processReplacing;
	return effect;
}

// this is required because GCC throws an error if we declare a non-standard function named 'main'
extern "C" __attribute__ ((visibility("default"))) AEffect * main_plugin(audioMasterCallback audioMaster) asm ("main");

extern "C" __attribute__ ((visibility("default"))) AEffect * main_plugin(audioMasterCallback audioMaster)
{
	return VSTPluginMain (audioMaster);
}
