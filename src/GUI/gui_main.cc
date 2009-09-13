
#include "gui_main.h"

#include "../AudioOutput.h"
#include "GUI.h"

static GUI *gui = NULL;
static Gtk::Main *kit = NULL;
static int gdk_input_pipe[2];

void gui_kit_init(int argc, char *argv[])
{
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
	
	gui = new GUI(config, midi_controller, vau, gdk_input_pipe[1], out, out->getTitle());
	gui->setPresetController(presetController);
	gui->init();
	
	// make GDK loop read events from the pipe
	gdk_input_add(gdk_input_pipe[0], GDK_INPUT_READ, &GUI::GdkInputFunction, NULL);
}

void ShowModalErrorMessage(const string & msg)
{
	Gtk::MessageDialog dlg ("amSynth", false, Gtk::MESSAGE_ERROR, Gtk::BUTTONS_OK, true);
	dlg.set_secondary_text(msg);
	dlg.run();
}

