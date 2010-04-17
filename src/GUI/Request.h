/* amSynth
 * (c) 2002-2006 Nick Dowell
 */

#ifndef _REQUEST_H
#define _REQUEST_H

#include <sigc++/slot.h>

#define CALL_ON_GUI_THREAD( object, method ) \
	call_slot_on_gui_thread( sigc::mem_fun((object), (method)) )

void call_slot_on_gui_thread( sigc::slot<void> sigc_slot );

#endif
