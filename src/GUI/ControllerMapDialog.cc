/* amSynth
 * (c) 2002 Nick Dowell
 */

#include "ControllerMapDialog.h"
#include <stdio.h>

ControllerMapDialog::ControllerMapDialog()
{
	set_title( "MIDI Controller Config" );
	
	table.resize( 32, 2 );
	add( table );
	
	for(int i=0; i<32; i++){
		string str( "MIDI Controller #" );
		char cstr[2];
		sprintf( cstr, "%d:", i );
		str += string( cstr );
		label[i].set_text( str );
		table.attach( label[i], 0, 1, i, i+1 );
		table.attach( combo[i], 1, 2, i, i+1 );
	}
}