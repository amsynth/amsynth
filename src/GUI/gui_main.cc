/*
 *  gui_main.cc
 *
 *  Copyright (c) 2001-2017 Nick Dowell
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

#include "MainWindow.h"


static char **_argv = NULL;

void gui_kit_init(int *argc, char ***argv)
{
	_argv = g_strdupv(*argv);
	gtk_init(argc, argv);
}

void gui_kit_run(unsigned (*timer_callback)())
{
	g_timeout_add(250, (GSourceFunc)timer_callback, NULL);
	gtk_main();
}

void gui_init(Synthesizer *synth, GenericOutput *out)
{
    gtk_window_set_default_icon_name("amsynth");

	GtkWidget *window = main_window_new(synth, out);
	gtk_widget_show_all(window);
}

void gui_dealloc()
{
//	if (gui) {
//		delete gui;
//		gui = NULL;
//	}
}

void ShowModalErrorMessage(const std::string & msg, const std::string & secondaryText)
{
	GtkWidget *dialog = gtk_message_dialog_new(
			NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
			"%s", msg.c_str());

	if (secondaryText.size())
		gtk_message_dialog_format_secondary_text(GTK_MESSAGE_DIALOG(dialog), "%s", secondaryText.c_str());

	gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);
}

////////////////////////////////////////////////////////////////////////////////

#if defined(__linux)

void spawn_new_instance()
{
	static char exe_path[4096] = "";
	readlink("/proc/self/exe", exe_path, sizeof(exe_path));
	_argv[0] = exe_path;
	g_spawn_async(NULL, _argv, NULL, (GSpawnFlags)0, NULL, NULL, NULL, NULL);
}

#endif
