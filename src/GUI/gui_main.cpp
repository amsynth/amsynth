/*
 *  gui_main.cpp
 *
 *  Copyright (c) 2001-2022 Nick Dowell
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

#include "gui_main.h"

#include <gtk/gtk.h>

#if defined(__linux)
#include <X11/Xlib.h>
#endif

static char **_argv = nullptr;

void gui_kit_init(int *argc, char ***argv)
{
#if defined(__linux)
	// Because JUCE uses a separate messaging thread.
	XInitThreads();
#endif
	_argv = g_strdupv(*argv);
	gtk_init(argc, argv);
	gtk_window_set_default_icon_name("amsynth");
}

#if defined(__linux)
namespace juce {
// Implemented in juce_linux_Messaging.cpp
	extern bool dispatchNextMessageOnSystemQueue(bool returnIfNoPendingMessages);
}
static gboolean on_idle(gpointer user_data)
{
	juce::dispatchNextMessageOnSystemQueue(true);
	return G_SOURCE_CONTINUE;
}
#endif

void gui_kit_run(unsigned (*timer_callback)(void *))
{
	g_idle_add(on_idle, nullptr);
	g_timeout_add(250, (GSourceFunc)timer_callback, nullptr);
	gtk_main();
}

void ShowModalErrorMessage(const std::string & msg, const std::string & secondaryText)
{
	GtkWidget *dialog = gtk_message_dialog_new(
			nullptr, GTK_DIALOG_MODAL, GTK_MESSAGE_ERROR, GTK_BUTTONS_OK,
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
	if (readlink("/proc/self/exe", exe_path, sizeof(exe_path)) == -1) {
		perror("readlink /proc/self/exe");
		return;
	}
	_argv[0] = exe_path;
	g_spawn_async(nullptr, _argv, nullptr, (GSpawnFlags)0, nullptr, nullptr, nullptr, nullptr);
}

#endif
