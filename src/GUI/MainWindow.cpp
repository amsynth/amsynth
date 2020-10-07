/*
 *  MainWindow.cpp
 *
 *  Copyright (c) 2001-2020 Nick Dowell
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

#include "MainWindow.h"

#include "../AudioOutput.h"
#include "../Configuration.h"
#include "../PresetController.h"
#include "../Synthesizer.h"

#include "ConfigDialog.h"
#include "editor_pane.h"
#include "gui_main.h"
#include "MainMenu.h"
#include "MIDILearnDialog.h"
#include "PresetControllerView.h"

#include <cmath>
#include <glib/gi18n.h>
#include <gtk/gtk.h>


static MIDILearnDialog *midiLearnDialog;


struct MainWindow : public UpdateListener
{
	MainWindow(Synthesizer *synthesizer, GenericOutput *audio, int scaling_factor) :
			synthesizer(synthesizer),
			presetController(synthesizer->getPresetController())
	{
		presetIsNotSaved = false;
		mainThread = g_thread_self();
		parameterUpdateQueue = g_async_queue_new();

		window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(window), PACKAGE_NAME);
		gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

		GtkAccelGroup *accelGroup = gtk_accel_group_new();
		gtk_window_add_accel_group(GTK_WINDOW(window), accelGroup);

		GtkWidget *vbox = gtk_vbox_new(FALSE, 0);

		//

		GtkWidget *menuBar = gtk_menu_bar_new();
		main_menu_init(window, accelGroup, GTK_MENU_BAR(menuBar), synthesizer);

		Configuration & config = Configuration::get();
		gchar *text = g_strdup_printf(_("Audio: %s @ %d  MIDI: %s"),
									  config.current_audio_driver.c_str(),
									  config.sample_rate,
									  config.current_midi_driver.c_str());
		GtkWidget *statusItem = gtk_menu_item_new_with_label(text);
		gtk_widget_set_sensitive(statusItem, FALSE);
		gtk_menu_item_set_right_justified(GTK_MENU_ITEM(statusItem), TRUE);
		gtk_menu_shell_append(GTK_MENU_SHELL(menuBar), statusItem);
		g_free(text);

		gtk_box_pack_start(GTK_BOX(vbox), menuBar, FALSE, FALSE, 0);

		//

		presetControllerView = PresetControllerView::instantiate(presetController);
		gtk_box_pack_start(GTK_BOX(vbox), presetControllerView->getWidget(), FALSE, FALSE, 0);

		//

		memset(defaults, 0, sizeof(defaults));

		presetController->setUpdateListener(*this);
		for (int i = 0; i < kAmsynthParameterCount; i++) {
			Preset &preset = presetController->getCurrentPreset();
			Parameter &parameter = preset.getParameter(i);
			adjustments[i] = (GtkAdjustment *) gtk_adjustment_new(
					parameter.getValue(),
					parameter.getMin(),
					parameter.getMax(),
					parameter.getStep(),
					0, 0);
			g_value_init(&defaults[i], G_TYPE_FLOAT);
			g_value_set_float(&defaults[i], parameter.getDefault());
			g_object_set_data(G_OBJECT(adjustments[i]), "default-value", &defaults[i]);
			parameter.addUpdateListener(this);
		}

		GtkWidget *editor = editor_pane_new(synthesizer, adjustments, FALSE, scaling_factor);
		gtk_box_pack_start(GTK_BOX(vbox), editor, FALSE, FALSE, 0);

		//
		// start_atomic_value_change is not registered by editor_pane_new
		//
		for (int i = 0; i < kAmsynthParameterCount; i++) {
			Preset &preset = presetController->getCurrentPreset();
			Parameter &parameter = preset.getParameter(i);

			g_object_set_data(G_OBJECT(adjustments[i]), "Parameter", &parameter);

			g_signal_connect_after(
					G_OBJECT(adjustments[i]), "start_atomic_value_change",
					G_CALLBACK(MainWindow::on_adjustment_start_atomic_value_change),
					(gpointer) this);

			g_signal_connect(
					G_OBJECT(adjustments[i]), "value_changed",
					G_CALLBACK(MainWindow::on_adjustment_value_changed),
					(gpointer) this);
		}

		//

		gtk_container_add(GTK_CONTAINER(window), vbox);
	}

	static void on_adjustment_start_atomic_value_change(GtkAdjustment *adjustment, MainWindow *mainWindow)
	{
		gdouble value = gtk_adjustment_get_value(adjustment);
		Parameter *parameter = (Parameter *) g_object_get_data(G_OBJECT(adjustment), "Parameter");
		mainWindow->presetController->pushParamChange(parameter->getId(), (float) value);
	}

	static void on_adjustment_value_changed(GtkAdjustment *adjustment, MainWindow *mainWindow)
	{
		gdouble value = gtk_adjustment_get_value(adjustment);
		Parameter *parameter = (Parameter *) g_object_get_data(G_OBJECT(adjustment), "Parameter");
		parameter->setValue((float) value);
	}

	void updateTitle()
	{
		Configuration & config = Configuration::get();
		std::ostringstream ostr;
		ostr << "amsynth";

		if (config.jack_client_name.length() && config.jack_client_name != "amsynth") {
			ostr << ": ";
			ostr << config.jack_client_name;
		}

		ostr << ": ";
		ostr << presetController->getCurrPresetNumber();

		ostr << ": ";
		ostr << presetController->getCurrentPreset().getName();

		if (presetIsNotSaved) {
			ostr << " *";
		}

		gtk_window_set_title(GTK_WINDOW(window), ostr.str().c_str());
	}

	bool shouldClose()
	{
		if (!presetIsNotSaved)
			return true;

		GtkWidget *dialog = gtk_message_dialog_new_with_markup(
				GTK_WINDOW(window),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_WARNING,
				GTK_BUTTONS_NONE,
				"<b>%s</b>", _("Save changes before closing?"));

		gtk_dialog_add_button(GTK_DIALOG(dialog), _("Close _Without Saving"), GTK_RESPONSE_NO);
		gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL);
		gtk_dialog_add_button(GTK_DIALOG(dialog), GTK_STOCK_SAVE, GTK_RESPONSE_YES);

		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", _("If you don't save, changes to the current preset will be permanently lost."));

		gint result = gtk_dialog_run (GTK_DIALOG (dialog));
		gtk_widget_destroy (dialog);

		if (result == GTK_RESPONSE_CANCEL) {
			return false;
		}

		if (result == GTK_RESPONSE_YES) {
			presetController->loadPresets(); // in case another instance has changed any of the other presets
			presetController->commitPreset();
			presetController->savePresets();
		}

		return true;
	}

	typedef std::pair<int, float> ParameterUpdate;

	void update() override
	{
		if (g_thread_self() == mainThread) {
			parameterDidChange(-1, NAN);
		} else {
			g_async_queue_push(parameterUpdateQueue, new ParameterUpdate(-1, NAN));
			g_idle_add(MainWindow::parameterUpdateIdleCallback, this);
		}
	}

	void UpdateParameter(Param paramID, float paramValue) override
	{
		if (g_thread_self() == mainThread) {
			parameterDidChange(paramID, paramValue);
		} else {
			g_async_queue_push(parameterUpdateQueue, new ParameterUpdate((int) paramID, paramValue));
			g_idle_add(MainWindow::parameterUpdateIdleCallback, this);
		}
	}

	static gboolean parameterUpdateIdleCallback(gpointer data)
	{
		MainWindow *mainWindow = (MainWindow *) data;

		ParameterUpdate *update;
		while ((update = (ParameterUpdate *) g_async_queue_try_pop(mainWindow->parameterUpdateQueue))) {
			mainWindow->parameterDidChange(update->first, update->second);
			delete update;
		}

		return G_SOURCE_REMOVE;
	}

	void parameterDidChange(int parameter, float value)
	{
		if (parameter == -1) {
			presetControllerView->update(); // note: PresetControllerView::update() is expensive
			presetIsNotSaved = presetController->isCurrentPresetModified();
			updateTitle();
			return;
		}
		if (0 <= parameter && parameter < kAmsynthParameterCount) {
			const Parameter &param = presetController->getCurrentPreset().getParameter(parameter);
			gtk_adjustment_set_value (adjustments[parameter], param.getValue());
		}
		bool isModified = presetController->isCurrentPresetModified();
		if (presetIsNotSaved != isModified) {
			presetIsNotSaved = isModified;
			updateTitle();
		}
	}

	GtkWidget *window;
	Synthesizer *synthesizer;
	PresetController *presetController;
	GenericOutput *audio;

	PresetControllerView *presetControllerView;
	GtkAdjustment *adjustments[kAmsynthParameterCount];
	GValue defaults[kAmsynthParameterCount];
	bool presetIsNotSaved;

	GThread *mainThread;
	GAsyncQueue *parameterUpdateQueue;
};


static gboolean
delete_event(GtkWidget *widget, GdkEvent *event, MainWindow *mainWindow)
{
	return mainWindow->shouldClose() ? FALSE : TRUE;
}

static gboolean
startup_check(gpointer data)
{
	Configuration & config = Configuration::get();
	bool bad_config = false;

	if (config.current_audio_driver.empty()) {
		bad_config = true;
		ShowModalErrorMessage(_("amsynth configuration error"),
							  _("amsynth could not initialise the selected audio device.\n\n"
								"Please review the configuration and restart"));
	}

	if (config.current_midi_driver.empty())  {
		bad_config = true;
		ShowModalErrorMessage(_("amsynth configuration error"),
							  _("amsynth could not initialise the selected MIDI device.\n\n"
								"Please review the configuration and restart"));
	}

	if (bad_config) {
		config_dialog_run(nullptr);
	}

    return G_SOURCE_REMOVE;
}

static GtkWidget *
main_window_new(Synthesizer *synthesizer, GenericOutput *audio, int scaling_factor)
{
	MainWindow *mainWindow = new MainWindow(synthesizer, audio, scaling_factor);
	g_signal_connect(G_OBJECT(mainWindow->window), "delete-event", G_CALLBACK(delete_event), mainWindow);
	g_signal_connect(G_OBJECT(mainWindow->window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

	midiLearnDialog = new MIDILearnDialog(
			synthesizer->getMidiController(),
			synthesizer->getPresetController(),
			GTK_WINDOW(mainWindow->window));

	g_idle_add(startup_check, mainWindow);

	return mainWindow->window;
}

void
main_window_show(Synthesizer *synthesizer, GenericOutput *audio, int scaling_factor)
{
	gtk_widget_show_all(main_window_new(synthesizer, audio, scaling_factor));
}

void
modal_midi_learn(Param param_index) // called by editor_pane upon right-clicking a control
{
	midiLearnDialog->run_modal(param_index);
}
