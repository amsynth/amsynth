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

/* To build the amsynth VST plugin;
 * 1) download the VST 2.4 SDK and unzip into the src/ directory
 *    you should have a "vstsdk2.4" directory in src now
 * 2) run: make -f Makefile.linux.vst
 *    this will produce amsynth.vst.so
 */

 /* Note:
  * Ardour's default search path is
  *   $HOME/.vst
  *   $PREFX/lib/vst
  */

#include "midi.h"
#include "Preset.h"
#include "Synthesizer.h"
#include "GUI/editor_pane.h"

#include <public.sdk/source/vst2.x/aeffeditor.h>
#include <public.sdk/source/vst2.x/audioeffectx.h>

#include <cassert>
#include <gdk/gdkx.h>
#include <gtk/gtk.h>
#include <stdio.h>
#include <X11/Xlib.h>

class Editor : public AEffEditor
{
public:
	Editor(AudioEffect *effect) : AEffEditor(effect)
	, mGdkParentWindow(0)
	, mGtkWindow(0)
	, mEditorWidget(0)
	{
	}

	~Editor()
	{
	}

	virtual bool getRect(ERect **rect)
	{
		mRect.top = 0;
		mRect.left = 0;
		mRect.right = 600;
		mRect.bottom = 400;
		*rect = &mRect;
		return true;
	}

	virtual bool open(void *ptr)
	{
		AEffEditor::open(ptr);

		static bool initialized = false;
		if (!initialized) {
			gtk_init(NULL, NULL);
			initialized = true;
		}

		if (!mEditorWidget) {
			for (int i = 0; i < kAmsynthParameterCount; i++) {
				gdouble value = 0, lower = 0, upper = 0, step_increment = 0;
				get_parameter_properties(i, &lower, &upper, &value, &step_increment);
				mAdjustments[i] = (GtkAdjustment *)gtk_adjustment_new(value, lower, upper, step_increment, 0, 0);
				g_object_ref_sink(mAdjustments[i]); // assumes ownership of the floating reference
				g_signal_connect(mAdjustments[i], "value-changed", (GCallback)&Editor::on_adjustment_value_changed, this);
			}
			mEditorWidget = editor_pane_new(mAdjustments, TRUE);
			g_object_ref_sink(mEditorWidget);
		}

		mGtkWindow = gtk_window_new(GTK_WINDOW_POPUP);

		// don't show the widget yet, to avoid visible moving of the window
		gtk_widget_realize(mGtkWindow); g_assert(gtk_widget_get_realized(mGtkWindow));

		// on some hosts (e.g. energyXT) creating the gdk window can fail unless we call gdk_display_sync
		gdk_display_sync(gdk_display_get_default());

		mGdkParentWindow = gdk_window_foreign_new((Window) systemWindow);
		g_assert(mGdkParentWindow);

		// use gdk_window_reparent instead of XReparentWindow to avoid "GdkWindow unexpectedly destroyed" warnings
		gdk_window_reparent(gtk_widget_get_window(mGtkWindow), mGdkParentWindow, 0, 0);

		gtk_container_add(GTK_CONTAINER(mGtkWindow), mEditorWidget);
		gtk_widget_show_all(mGtkWindow);

		gdk_display_sync(gdk_display_get_default());

		return 0;
	}

	virtual void close()
	{
		if (mGtkWindow) {
			if (gtk_widget_get_window(mGtkWindow)) {
				gdk_window_hide(gtk_widget_get_window(mGtkWindow));
			}
			gtk_widget_destroy(mGtkWindow);
		}

		mGdkParentWindow = NULL;
		mGtkWindow = NULL;
		mEditorWidget = NULL;

		for (int i = 0; i < kAmsynthParameterCount; i++) {
			mAdjustments[i] = NULL;
		}

		gdk_display_sync(gdk_display_get_default());

		AEffEditor::close();
	}

	virtual void idle()
	{
		while (gtk_events_pending())
			gtk_main_iteration();
	}

	static void on_adjustment_value_changed(GtkAdjustment *adjustment, gpointer user_data)
	{
		Editor *editor = (Editor *)user_data;

		static Preset dummyPreset;

		for (int i = 0; i < kAmsynthParameterCount; i++) {
			if (adjustment == editor->mAdjustments[i]) {
				float value = gtk_adjustment_get_value(adjustment);
				Parameter &param = dummyPreset.getParameter(i);
				param.setValue(value);
				editor->getEffect()->setParameterAutomated(i, param.GetNormalisedValue());
			}
		}
	}

private:
	ERect mRect;
	GdkWindow *mGdkParentWindow;
	GtkWidget *mGtkWindow;
	GtkWidget *mEditorWidget;
	GtkAdjustment *mAdjustments[kAmsynthParameterCount];
};


class AmsynthVST : public AudioEffectX
{
public:
	
	AmsynthVST(audioMasterCallback callback) : AudioEffectX(callback, 0, kAmsynthParameterCount)
	{
		setUniqueID(CCONST('a', 'm', 's', 'y'));
		setNumInputs(0);
		setNumOutputs(2);
		canProcessReplacing(true);
		isSynth(true);

		mMidiBuffer = (unsigned char *)malloc(4096);
		mSynthesizer = new Synthesizer;
		
		setEditor(mEditor = new Editor(this));
	}

	~AmsynthVST()
	{
		free(mMidiBuffer);
		delete mSynthesizer;
	}

	virtual bool getEffectName(char *name)
	{
		strcpy(name, "amsynth"); return true;
	}

	virtual bool getVendorString(char *text)
	{
		strcpy(text, "Nick Dowell"); return true;
	}

	virtual bool getProductString(char *text)
	{
		strcpy(text, "amsynth"); return true;
	}

	virtual void setParameter(VstInt32 index, float value)
	{
		mSynthesizer->setNormalizedParameterValue((Param) index, value);
	}
	
	virtual float getParameter(VstInt32 index)
	{
		return mSynthesizer->getNormalizedParameterValue((Param) index);
	}
	
	virtual void getParameterLabel(VstInt32 index, char *text)
	{
		mSynthesizer->getParameterLabel((Param) index, text, 32);
	}
	
	virtual void getParameterDisplay(VstInt32 index, char *text)
	{
		mSynthesizer->getParameterDisplay((Param) index, text, 32);
	}
	
	virtual void getParameterName(VstInt32 index, char *text)
	{
		mSynthesizer->getParameterName((Param) index, text, 32);
	}

	virtual VstInt32 processEvents(VstEvents *events)
	{
		assert(mMidiEvents.empty());

		memset(mMidiBuffer, 0, 4096);
		unsigned char *buffer = mMidiBuffer;
		
		for (VstInt32 i=0; i<events->numEvents; i++) {
			if (events->events[i]->type == kVstMidiType) {
				VstMidiEvent *event = (VstMidiEvent *)events->events[i];

				memcpy(buffer, event->midiData, 4);

				amsynth_midi_event_t midi_event;
				memset(&midi_event, 0, sizeof(midi_event));
				midi_event.offset_frames = event->deltaFrames;
				midi_event.buffer = buffer;
				midi_event.length = 4;
				mMidiEvents.push_back(midi_event);

				buffer += event->byteSize;

				assert(buffer < mMidiBuffer + 4096);
			}
		}

		return 1;
	}
	
	virtual void processReplacing(float **inputs, float **outputs, VstInt32 numSampleFrames)
	{
		mSynthesizer->process(numSampleFrames, mMidiEvents, outputs[0], outputs[1]);
		mMidiEvents.clear();
	}
	
private:
	
	std::vector<amsynth_midi_event_t> mMidiEvents;
	unsigned char *mMidiBuffer;
	Synthesizer *mSynthesizer;
	Editor *mEditor;
};


void modal_midi_learn(int param_index) {}


extern "C" AEffect * VSTPluginMain(audioMasterCallback audioMaster)
{
	AudioEffectX *plugin = new AmsynthVST(audioMaster);
	if (!plugin) {
		return NULL;
	}
	return plugin->getAeffect();
}

// this is required because GCC throws an error if we declare a non-standard function named 'main'
extern "C" __attribute__ ((visibility("default"))) AEffect * main_plugin(audioMasterCallback audioMaster) asm ("main");

extern "C" __attribute__ ((visibility("default"))) AEffect * main_plugin(audioMasterCallback audioMaster)
{
	return VSTPluginMain (audioMaster);
}

#include <public.sdk/source/vst2.x/audioeffect.cpp>
#include <public.sdk/source/vst2.x/audioeffectx.cpp>
