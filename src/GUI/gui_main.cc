
#include "gui_main.h"

#include "../AudioOutput.h"
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

void gui_kit_run()
{
	kit->run();
}

void gui_init(Config &config,
              MidiController &midi_controller,
              VoiceAllocationUnit &vau,
              PresetController &presetController,
              GenericOutput *out)
{
	if (pipe(gdk_input_pipe) == -1)
		perror("pipe()");
	
	gui = new GUI(config, midi_controller, vau, out, out->getTitle());
	gui->setPresetController(presetController);
	gui->init();
	
	// make GDK loop read events from the pipe
	gdk_input_add(gdk_input_pipe[0], GDK_INPUT_READ, gdk_input_function, NULL);
}

void ShowModalErrorMessage(const string & msg)
{
	Gtk::MessageDialog dlg ("amSynth", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
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
	g_spawn_async(NULL, _argv, NULL, (GSpawnFlags)0, NULL, NULL, NULL, NULL);
}
