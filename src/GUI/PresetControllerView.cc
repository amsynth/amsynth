/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "PresetControllerView.h"
#include <stdio.h>

using SigC::slot;
using SigC::bind;

PresetControllerView::PresetControllerView( int pipe_d, VoiceAllocationUnit & vau )
{
	this->vau = &vau;
	inhibit_combo_callback = false;
	inhibit_combo_update = false;
	
    commit.add_label("Save Changes");
    commit.clicked.connect(
		bind <char*>(slot(this, &PresetControllerView::ev_handler),"commit"));

	presets_combo.get_entry()->set_editable( false );
	presets_combo.get_entry()->changed.connect(
		bind <char*>(slot(this, &PresetControllerView::ev_handler),"presets_combo"));

    add( preset_no_entry );
    add( presets_combo );
	add( commit );

	piped = pipe_d;
	request.slot = slot( this, &PresetControllerView::_update_ );
}

PresetControllerView::~PresetControllerView()
{
}

void
PresetControllerView::setPresetController(PresetController & p_c)
{
    presetController = &p_c;
    p_c.setUpdateListener(*this);
    update();
}

void 
PresetControllerView::ev_handler(string text)
{
	if (text == "commit") {
		presetController->commitPreset();
		update();
		return;
	} else if (text == "presets_combo") {
		if (inhibit_combo_callback==false){
			inhibit_combo_update = true;
			string preset_name = presets_combo.get_entry()->get_text();
			int foo = presetController->selectPreset( preset_name );
			vau->killAllVoices();
			inhibit_combo_update = false;
		} else
		return;
	} else {
#ifdef _DEBUG
		cout << "<PresetController::ev_handler> couldnt find action for '"
		<< text << " '" << endl;
#endif
		return;
    }
}

void
PresetControllerView::update()
{
	if(!inhibit_combo_callback)
		if( write( piped, &request, sizeof(request) ) != sizeof(request) )
			cout << "ParameterSwitch: error writing to pipe" << endl;
}

void 
PresetControllerView::_update_()
{
	inhibit_combo_callback = true;
	
	// update our list of preset names
	if(inhibit_combo_update==false){
		inhibit_combo_callback = true;
		list<string> gl;
		for (int preset=0; preset<PRESETS; preset++){
			string preset_name = presetController->getPreset(preset).getName();
			if ( preset_name != "New Preset" ) gl.push_back( preset_name );
		}
		// set the popdown list of preset names
		presets_combo.set_popdown_strings( gl );
		presets_combo.get_entry()->set_text(presetController->getCurrentPreset().getName());
		inhibit_combo_callback = false;
	}
	
	// set the display entries
    char cstr[3];
    sprintf(cstr, "%d", presetController->getCurrPresetNumber());
	string txt("Preset ");
	txt += string(cstr);
	txt += " : ";
    preset_no_entry.set_text(txt);	
	
	inhibit_combo_callback = false;
}