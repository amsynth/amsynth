/* amSynth
 * (c) 2002 Nick Dowell
 */

#include "AboutDialog.h"

#include "splash.xpm"

AboutDialog::AboutDialog()
{
	set_title( "About" );

	about_pixmap = new Gtk::Pixmap( splash_xpm );
	get_vbox()->add( *about_pixmap );
	
    get_action_area()->add( about_close_button );

    about_close_button.add_label( "close", 0.5, 0.5 );
    about_close_button.clicked.connect( hide.slot() );
}