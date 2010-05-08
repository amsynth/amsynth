
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

void ShowModalErrorMessage(const std::string & msg);

