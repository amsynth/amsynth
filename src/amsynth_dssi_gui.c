//
//  amsynth_dssi_gui.c
//  amsynth_dssi_gui
//
//  Created by Nick Dowell on 04/06/2011.
//  Copyright 2011 __MyCompanyName__. All rights reserved.
//

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <gtk/gtk.h>
#include <lo/lo.h>

////////////////////////////////////////////////////////////////////////////////

#define MAX_PATH 160

static GtkWindow *_window = NULL;

static char *_osc_path = NULL;
lo_server _osc_server = NULL;
lo_address _osc_host_addr = NULL;

////////////////////////////////////////////////////////////////////////////////

//
// convenience function that allocates a formatted string
// the returned string is only valid until the next call to the function
// so be sure to copy the result if you need to use it beyond that!
// not at all thread safe, and probably a bad idea...!
//
char *tmpstr(const char *format, ...)
{
    static char *string = NULL;
    
    if (string) {
        free(string);
        string = NULL;
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

////////////////////////////////////////////////////////////////////////////////
//
// handle message sent by plugin host
//

int osc_control_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    assert(types[0] == 'i');
    assert(types[1] == 'f');
    printf("OSC: control %2d = %f\n", argv[0]->i, argv[1]->f);
    return 0;
}

int osc_samplerate_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    assert(types[0] == 'i');
    printf("OSC: sample rate = %d\n", argv[0]->i);
    return 0;
}

int osc_program_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    assert(types[0] == 'i');
    assert(types[1] == 'i');
    printf("OSC: selected bank %d program %2d\n", argv[0]->i, argv[1]->i);
    return 0;
}

int osc_show_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    printf("OSC: show GUI window\n");
    gtk_window_present(_window);
    return 0;
}

int osc_hide_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    printf("OSC: hide GUI window\n");
    gtk_widget_hide(GTK_WIDGET(_window));
    return 0;
}

int osc_quit_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    printf("OSC: quit GUI process\n");
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
    lo_arg data[2]; data[0].i = control; data[1].f = value;
    int err = lo_send(_osc_host_addr, tmpstr("%s/control", _osc_path), "if", data[0], data[1]);
    return err;
}

int host_set_program(int bank, int program)
{
    lo_arg data[2]; data[0].i = bank; data[1].i = program;
    int err = lo_send(_osc_host_addr, tmpstr("%s/program", _osc_path), "ii", data[0], data[1]);
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
    _window = NULL;
    host_gui_exiting();
    gtk_main_quit();
}

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    gtk_init(&argc, &argv);

    char *exe_path = argv[0];
    char *host_url = argv[1];
    char *lib_name = argv[2];
    char *plug_name = argv[3];
    char *identifier = argv[4];
    
    _osc_path = lo_url_get_path(host_url);
    _osc_host_addr = lo_address_new_from_url(host_url);
    
    _osc_server = lo_server_new(NULL, osc_error);
    lo_server_add_method(_osc_server, tmpstr("/%s/control",     _osc_path), "if", osc_control_handler,     NULL);
    lo_server_add_method(_osc_server, tmpstr("/%s/sample-rate", _osc_path), "i",  osc_samplerate_handler,  NULL);
    lo_server_add_method(_osc_server, tmpstr("/%s/program",     _osc_path), "ii", osc_program_handler,     NULL);
    lo_server_add_method(_osc_server, tmpstr("/%s/show",        _osc_path), NULL, osc_show_handler,        NULL);
    lo_server_add_method(_osc_server, tmpstr("/%s/hide",        _osc_path), NULL, osc_hide_handler,        NULL);
    lo_server_add_method(_osc_server, tmpstr("/%s/quit",        _osc_path), NULL, osc_quit_handler,        NULL);
    lo_server_add_method(_osc_server, NULL, NULL, osc_fallback_handler, NULL);
    
    host_request_update();
    
    gdk_input_add(lo_server_get_socket_fd(_osc_server), GDK_INPUT_READ, (GdkInputFunction)lo_server_recv_noblock, _osc_server);
    
    _window = GTK_WINDOW(gtk_window_new(GTK_WINDOW_TOPLEVEL));

    gtk_window_set_title(_window, tmpstr("%s - %s", plug_name, identifier));
    
	gtk_signal_connect(GTK_OBJECT(_window), "delete-event", on_window_deleted, NULL);
    
    gtk_main();
    
    return 0;
}

////////////////////////////////////////////////////////////////////////////////
