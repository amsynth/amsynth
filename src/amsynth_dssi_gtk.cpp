/*
 *  amsynth_dssi_gtk.cpp
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

#include "amsynth_dssi.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <gdk/gdk.h>
#include <gtk/gtk.h>
#include <lo/lo.h>

#include "controls.h"
#include "Preset.h"
#include "Synthesizer.h"
#include "GUI/editor_pane.h"

////////////////////////////////////////////////////////////////////////////////

#define MAX_PATH 160

static GtkWindow *_window = nullptr;
static GtkAdjustment *_adjustments[kAmsynthParameterCount] = {nullptr};
static gboolean _dont_send_control_changes = FALSE;

static char *_osc_path = nullptr;
lo_server _osc_server = nullptr;
lo_address _osc_host_addr = nullptr;

////////////////////////////////////////////////////////////////////////////////

//
// convenience function that allocates a formatted string
// the returned string is only valid until the next call to the function
// so be sure to copy the result if you need to use it beyond that!
// not at all thread safe, and probably a bad idea...!
//
char *tmpstr(const char *format, ...)
{
    static char *string = nullptr;
    
    if (string) {
        free(string);
        string = nullptr;
    }
    
    va_list args;
    va_start(args, format);
    vasprintf(&string, format, args);
    va_end(args);
    
    return string;
}

void osc_error(int num, const char *msg, const char *path)
{
    abort();
}

static gboolean osc_input_handler(GIOChannel *source, GIOCondition condition, gpointer data)
{
    lo_server_recv_noblock(_osc_server, 0);
    return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
//
// handle message sent by plugin host
//

int osc_control_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    assert(types[0] == 'i');
    assert(types[1] == 'f');
    float value = argv[1]->f;
    int port_number = argv[0]->i;
    int parameter_index = port_number - 2;
    g_assert(parameter_index < kAmsynthParameterCount);
    _dont_send_control_changes = TRUE;
    gtk_adjustment_set_value(_adjustments[parameter_index], value);
    _dont_send_control_changes = FALSE;
    return 0;
}

int osc_samplerate_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    assert(types[0] == 'i');
    return 0;
}

int osc_program_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    assert(types[0] == 'i');
    assert(types[1] == 'i');
    return 0;
}

int osc_show_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    gtk_window_present(_window);
    return 0;
}

int osc_hide_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    gtk_widget_hide(GTK_WIDGET(_window));
    return 0;
}

int osc_quit_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    gtk_main_quit();
    return 0;
}

int osc_fallback_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    fprintf(stderr, "unhandled OSC message (path = '%s' types = '%s')\n", path, types);
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
//
// Messages sent to the plugin host
//

int host_request_update()
{
    char value[MAX_PATH] = "";
    char *url = lo_server_get_url(_osc_server);
    sprintf(value, "%s%s", url, _osc_path);
    int err = lo_send(_osc_host_addr, tmpstr("%s/update", _osc_path), "s", value);
    free(url);
    return err;
}

int host_set_control(int control, float value)
{
    int err = lo_send(_osc_host_addr, tmpstr("%s/control", _osc_path), "if", control, value);
    return err;
}

int host_configure(const char *key, const char *value)
{
    int err = lo_send(_osc_host_addr, tmpstr("%s/configure", _osc_path), "ss", key, value);
    return err;
}

int host_gui_exiting()
{
    int err = lo_send(_osc_host_addr, tmpstr("%s/exiting", _osc_path), "");
    return err;
}

////////////////////////////////////////////////////////////////////////////////

void on_window_deleted()
{
    _window = nullptr;
    host_gui_exiting();
    gtk_main_quit();
}

void on_adjustment_value_changed(GtkAdjustment *adjustment, gpointer user_data)
{
    if (_dont_send_control_changes)
        return;
    size_t parameter_index = (size_t)user_data;
    g_assert(parameter_index < kAmsynthParameterCount);
    int port_number = parameter_index + 2;
    host_set_control(port_number, gtk_adjustment_get_value(adjustment));
}

////////////////////////////////////////////////////////////////////////////////

struct SynthesizerStub : ISynthesizer
{
	int loadTuningKeymap(const char *filename) override
	{
		return host_configure(PROP_KBM_FILE, filename);
	}

	int loadTuningScale(const char *filename) override
	{
		return host_configure(PROP_SCL_FILE, filename);
	}
};

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    if (argc < 5) {
        g_critical("not enough arguments supplied");
        return 1;
    }

    gtk_init(&argc, &argv);

    //char *exe_path = argv[0];
    char *host_url = argv[1];
    //char *lib_name = argv[2];
    char *plug_name = argv[3];
    char *identifier = argv[4];
    
    _osc_path = lo_url_get_path(host_url);
    _osc_host_addr = lo_address_new_from_url(host_url);
    
    _osc_server = lo_server_new(nullptr, osc_error);
    lo_server_add_method(_osc_server, tmpstr("/%s/control",     _osc_path), "if",    osc_control_handler,     nullptr);
    lo_server_add_method(_osc_server, tmpstr("/%s/sample-rate", _osc_path), "i",     osc_samplerate_handler,  nullptr);
    lo_server_add_method(_osc_server, tmpstr("/%s/program",     _osc_path), "ii",    osc_program_handler,     nullptr);
    lo_server_add_method(_osc_server, tmpstr("/%s/show",        _osc_path), nullptr, osc_show_handler,        nullptr);
    lo_server_add_method(_osc_server, tmpstr("/%s/hide",        _osc_path), nullptr, osc_hide_handler,        nullptr);
    lo_server_add_method(_osc_server, tmpstr("/%s/quit",        _osc_path), nullptr, osc_quit_handler,        nullptr);
    lo_server_add_method(_osc_server, nullptr, nullptr, osc_fallback_handler, nullptr);
    
    host_request_update();

    g_io_add_watch(g_io_channel_unix_new(lo_server_get_socket_fd(_osc_server)), G_IO_IN, osc_input_handler, nullptr);
    
    _window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));

    gtk_window_set_title(_window, tmpstr("%s - %s", plug_name, identifier));
    g_signal_connect(GTK_OBJECT(_window), "delete-event", on_window_deleted, NULL);

    size_t i; for (i=0; i<kAmsynthParameterCount; i++) {
        gdouble value = 0, lower = 0, upper = 0, step_increment = 0;
        get_parameter_properties(i, &lower, &upper, &value, &step_increment);
        _adjustments[i] = (GtkAdjustment *)gtk_adjustment_new(value, lower, upper, step_increment, 0, 0);
        g_signal_connect(_adjustments[i], "value-changed", (GCallback)&on_adjustment_value_changed, (gpointer)i);
    }

    GtkWidget *editor = editor_pane_new(new SynthesizerStub, _adjustments, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(_window), editor);
    gtk_widget_show_all(GTK_WIDGET(editor));
    
    gtk_main();
    
    return 0;
}

void modal_midi_learn(Param param_index)
{
}

////////////////////////////////////////////////////////////////////////////////

