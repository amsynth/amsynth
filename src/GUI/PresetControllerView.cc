/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "PresetControllerView.h"
#include "../PresetController.h"
#include "../VoiceAllocationUnit.h"
#include <stdio.h>
#include <iostream>

using SigC::slot;
using sigc::bind;
using std::cout;

PresetControllerView::PresetControllerView( int pipe_d, VoiceAllocationUnit & vau )
{
	this->vau = &vau;
	inhibit_combo_callback = false;
	inhibit_combo_update = false;
	
    commit.add_label("Save Changes",0.5);
    commit.signal_clicked().connect(
		bind <char*>(mem_fun(*this, &PresetControllerView::ev_handler),"commit"));

	presets_combo.get_entry()->set_editable( false );
	presets_combo.get_entry()->signal_changed().connect(
		bind <char*>(mem_fun(*this, &PresetControllerView::ev_handler),"presets_combo"));

    add( preset_no_entry );
    add( presets_combo );
	add( commit );

	Gtk::Label *blank = manage (new Gtk::Label ("    "));
	add (*blank);

	Gtk::Button *panic = manage (new Gtk::Button);
	panic->add_label ("Panic");
	panic->signal_clicked().connect(bind(mem_fun(*this, &PresetControllerView::ev_handler),"panic"));
	add (*panic);

	piped = pipe_d;
	request.slot = mem_fun(*this, &PresetControllerView::_update_ );
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
			vau->killAllVoices();
			presetController->selectPreset( preset_name );
			inhibit_combo_update = false;
		} else
		return;
	} else if (text == "panic") {
		vau->killAllVoices();
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
