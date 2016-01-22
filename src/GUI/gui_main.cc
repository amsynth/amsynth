/*
 *  gui_main.cc
 *
 *  Copyright (c) 2001-2016 Nick Dowell
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

#include "gui_main.h"

#include "../AudioOutput.h"
#include "../Synthesizer.h"
#include "GUI.h"

#include <assert.h>

static GUI *gui = NULL;
static Gtk::Main *kit = NULL;
static int  gdk_input_pipe[2];
static void gdk_input_function(gpointer, gint, GdkInputCondition);
static char **_argv = NULL;

void gui_kit_init(int & argc, char ** & argv)
{
	_argv = g_strdupv(argv);
	kit = new Gtk::Main(argc, argv);
}

void gui_kit_run(unsigned (*timer_callback)())
{
	g_timeout_add(250, (GSourceFunc)timer_callback, NULL);
	kit->run();
}

void gui_init(Synthesizer *synth, GenericOutput *out)
{
	if (pipe(gdk_input_pipe) == -1)
		perror("pipe()");
    
    gtk_window_set_default_icon_name("amsynth");
	
	gui = new GUI(*synth->getMidiController(), synth, out);
	gui->setPresetController(*synth->getPresetController());
	gui->init();
	
	// make GDK loop read events from the pipe
	gdk_input_add(gdk_input_pipe[0], GDK_INPUT_READ, gdk_input_function, NULL);
}

void gui_dealloc()
{
	if (gui) {
		delete gui;
		gui = NULL;
	}
}

void ShowModalErrorMessage(const string & msg)
{
	Gtk::MessageDialog dlg ("amsynth", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
	dlg.set_secondary_text(msg);
	dlg.run();
}

////////////////////////////////////////////////////////////////////////////////

typedef struct { sigc::slot<void> slot; } Request;

void call_slot_on_gui_thread( sigc::slot<void> sigc_slot )
{
	Request *req = new Request; req->slot = sigc_slot;
	ssize_t bytesWritten = write(gdk_input_pipe[1], &req, sizeof(req));
	assert(bytesWritten == sizeof(req));
}

void gdk_input_function(gpointer, gint source, GdkInputCondition)
{
	Request *request = NULL;
	ssize_t bytesRead = read(source, &request, sizeof(request));
	assert(bytesRead == sizeof(request));
	assert(request != NULL);
	
	if (bytesRead == sizeof(request) && request != NULL) {
		request->slot();
		delete request;
	}
}

void spawn_new_instance()
{
#ifdef linux
	static char exe_path[4096] = "";
	readlink("/proc/self/exe", exe_path, sizeof(exe_path));
	_argv[0] = exe_path;
#endif
	g_spawn_async(NULL, _argv, NULL, (GSpawnFlags)0, NULL, NULL, NULL, NULL);
}
