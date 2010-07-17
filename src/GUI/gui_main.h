
#include <string>

class Config;
class GenericOutput;
class MidiController;
class PresetController;
class VoiceAllocationUnit;

// Gtk::Main()
void gui_kit_init(int & argc, char ** & argv);

// Gtk::Main::run()
void gui_kit_run();

void gui_init(Config &,
              MidiController &,
              VoiceAllocationUnit &,
              PresetController &,
              GenericOutput *);

void gui_dealloc();

void ShowModalErrorMessage(const std::string & msg);

