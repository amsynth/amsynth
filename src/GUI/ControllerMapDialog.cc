/* amSynth
 * (c) 2002 Nick Dowell
 */

#include "ControllerMapDialog.h"
#include <stdio.h>
#include <list>

using SigC::slot;
using SigC::bind;

ControllerMapDialog::ControllerMapDialog( MidiController & mc, PresetController & pc )
{
	midi_controller = &mc;
	preset_controller = &pc;
	
	set_title( "MIDI Controller Config" );
	
	table.resize( 32, 2 );
	add( table );
	
	list<string> gl;
	for (gint p=0; p<128; p++){
		string p_name = preset_controller->getCurrentPreset().getParameter(p).getName();
		if ( p_name != "unused" ) gl.push_back( p_name );
	}
	
	for(gint i=0; i<32; i++){
		string str( "MIDI Controller #" );
		char cstr[2];
		sprintf( cstr, "%d:", i );
		str += string( cstr );
		label[i].set_text( str );
		table.attach( label[i], 0, 1, i, i+1 );
		combo[i].set_popdown_strings( gl );
		combo[i].get_entry()->set_editable( false );
		combo[i].get_entry()->changed.connect(
			bind(slot(this, &ControllerMapDialog::callback),i) );
		table.attach( combo[i], 1, 2, i, i+1 );
	}

	_update_();
}

void
ControllerMapDialog::_update_()
{
	supress_callback = true;
	for(int i=0; i<32; i++)
		combo[i].get_entry()->set_text( midi_controller->getController(i).getName() );
	supress_callback = false;
}

void
ControllerMapDialog::callback( gint cc )
{
	if(!supress_callback){
		// cc is the combo box which triggered the callback
		cout << "combo box " << cc << " changed to " << combo[cc].get_entry()->get_text() << endl;
		midi_controller->setController( 
			cc, preset_controller->getCurrentPreset().getParameter(combo[cc].get_entry()->get_text()) );
	}}