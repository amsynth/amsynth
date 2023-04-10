/*
 *  dssiui.cpp
 *
 *  Copyright (c) 2001-2023 Nick Dowell
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

#include "config.h"
#include "core/controls.h"
#include "core/gui/MainComponent.h"
#include "core/synth/PresetController.h"
#include "core/synth/Synthesizer.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <lo/lo.h>

////////////////////////////////////////////////////////////////////////////////

#define MAX_PATH 160

juce::TopLevelWindow *mainWindow;
juce::String windowTitle;
PresetController presetController;

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
#ifdef __GNUC__
static char *tmpstr(const char *format, ...) __attribute__ ((format(printf, 1, 2)));
#endif

static char *tmpstr(const char *format, ...)
{
    static char *string = nullptr;
    
    if (string) {
        free(string);
        string = nullptr;
    }
    
    va_list args;
    va_start(args, format);
    int res = vasprintf(&string, format, args);
    va_end(args);
    
    if (res == -1) {
       return nullptr;
    }

    return string;
}

static void osc_error(int num, const char *msg, const char *path)
{
    fprintf(stderr, "OSC error (num = %d msg = '%s' path = '%s')\n", num, msg, path);
}

////////////////////////////////////////////////////////////////////////////////
//
// handle message sent by plugin host
//

static int osc_control_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    assert(types[0] == 'i');
    assert(types[1] == 'f');
    float value = argv[1]->f;
    int port_number = argv[0]->i;
    int parameter_index = port_number - 2;
    presetController.getCurrentPreset().getParameter(parameter_index).setValue(value);
    return 0;
}

static int osc_samplerate_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    assert(types[0] == 'i');
    return 0;
}

static int osc_program_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    assert(types[0] == 'i');
    assert(types[1] == 'i');
    return 0;
}

static int osc_show_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    mainWindow->setVisible(true);
    return 0;
}

static int osc_hide_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    mainWindow->setVisible(false);
    return 0;
}

static int osc_quit_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
    return 0;
}

static int osc_fallback_handler(const char *path, const char *types, lo_arg **argv, int argc, void *data, void *user_data)
{
    fprintf(stderr, "unhandled OSC message (path = '%s' types = '%s')\n", path, types);
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
//
// Messages sent to the plugin host
//

static int host_request_update()
{
    char value[MAX_PATH] = "";
    char *url = lo_server_get_url(_osc_server);
    sprintf(value, "%s%s", url, _osc_path);
    int err = lo_send(_osc_host_addr, tmpstr("%s/update", _osc_path), "s", value);
    free(url);
    return err;
}

static int host_set_control(int control, float value)
{
    int err = lo_send(_osc_host_addr, tmpstr("%s/control", _osc_path), "if", control, value);
    return err;
}

static int host_configure(const char *key, const char *value)
{
    int err = lo_send(_osc_host_addr, tmpstr("%s/configure", _osc_path), "ss", key, value);
    return err;
}

static int host_gui_exiting()
{
    int err = lo_send(_osc_host_addr, tmpstr("%s/exiting", _osc_path), "");
    return err;
}

////////////////////////////////////////////////////////////////////////////////

class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow() : DocumentWindow(windowTitle, juce::Colours::lightgrey,
                                  juce::DocumentWindow::closeButton |
                                  juce::DocumentWindow::minimiseButton)
    {
        setUsingNativeTitleBar(true);
        auto component = new MainComponent(&presetController);
        component->loadTuningKbm = [] (const char *file) {
            host_configure(PROP_KBM_FILE, file);
        };
        component->loadTuningScl = [] (const char *file) {
            host_configure(PROP_SCL_FILE, file);
        };
        setContentOwned(component, true);
        centreWithSize(getWidth(), getHeight());
        setResizable(false, false);
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

////////////////////////////////////////////////////////////////////////////////

class Application : public juce::JUCEApplication
{
public:
    void initialise(const juce::String &commandLine) override
    {
        juce::LinuxEventLoop::registerFdCallback(lo_server_get_socket_fd(_osc_server), [] (int fd) {
            lo_server_recv_noblock(_osc_server, 0);
        });
        mainWindow = new MainWindow();
    }

    void shutdown() override
    {
        host_gui_exiting();
    }

    const juce::String getApplicationName() override
    {
        return windowTitle;
    }

    const juce::String getApplicationVersion() override
    {
        return PACKAGE_VERSION;
    }
};

static juce::JUCEApplicationBase * create_application()
{
    return new Application;
}

////////////////////////////////////////////////////////////////////////////////

struct ParameterListener : UpdateListener
{
    void update() override {};

    void UpdateParameter(Param param, float controlValue) override
    {
        int port_number = param + 2;
        auto value = presetController.getCurrentPreset().getParameter(param).getValue();
        host_set_control(port_number, value);
    }
};

////////////////////////////////////////////////////////////////////////////////

int main(int argc, char *argv[])
{
    if (argc < 5) {
        fprintf(stderr, "not enough arguments supplied\n");
        return 1;
    }

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

    windowTitle = tmpstr("%s - %s", plug_name, identifier);

    presetController.getCurrentPreset().AddListenerToAll(new ParameterListener);

    juce::JUCEApplicationBase::createInstance = &create_application;
    return juce::JUCEApplicationBase::main(JUCE_MAIN_FUNCTION_ARGS);
}

void modal_midi_learn(Param param_index)
{
}

////////////////////////////////////////////////////////////////////////////////

