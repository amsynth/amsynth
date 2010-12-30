
#include "../UpdateListener.h"

class MidiController;
class PresetController;

class MIDILearnDialog : public UpdateListener
{
public:

	MIDILearnDialog(MidiController *midiController, PresetController *presetController, GtkWindow *parent);
	~MIDILearnDialog();

	void run_modal(unsigned param_idx);	

private:

	virtual void update();

	void last_active_controller_changed();

	GtkWidget		*_dialog;
	GtkWidget		*_paramNameEntry;
	GtkWidget	*_ccSpinButton;

	MidiController	*_midiController;
	PresetController *_presetController;
};

