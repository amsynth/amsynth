/* amSynth
 * (c) 2002 Nick Dowell
 */

#include "ControllerMapDialog.h"
#include <stdio.h>
#include <list>

using SigC::slot;
using SigC::bind;

ControllerMapDialog::ControllerMapDialog( int pipe_d, MidiController & mc, 
											PresetController & pc )
{
	piped = pipe_d;
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
		table.attach( active[i], 0, 1, i, i+1 );
		table.attach( label[i], 1, 2, i, i+1 );
		combo[i].set_popdown_strings( gl );
		combo[i].get_entry()->set_editable( false );
		combo[i].get_entry()->changed.connect(
			bind(slot(this, &ControllerMapDialog::callback),i) );
		table.attach( combo[i], 2, 3, i, i+1 );
	}

	request.slot = slot( this, &ControllerMapDialog::_updateActive_ );
	
	midi_controller->getLastControllerParam().addUpdateListener( *this );
	
	_update_();
	_updateActive_();
}

void
ControllerMapDialog::update()
{
	if( write( piped, &request, sizeof(request) ) != sizeof(request) )
		cout << "ParameterSwitch: error writing to pipe" << endl;
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
ControllerMapDialog::_updateActive_()
{
	int i;
	int lastactive = (int)midi_controller->getLastControllerParam().getValue();
	for( i=0; i<32; i++ ){
		if( i==lastactive ) active[i].set_text( "<> " );
		else active[i].set_text( "" );
	}
}

void
ControllerMapDialog::callback( gint cc )
{
	if(!supress_callback)
		midi_controller->setController( 
			cc, preset_controller->getCurrentPreset().getParameter(combo[cc].get_entry()->get_text()) );}