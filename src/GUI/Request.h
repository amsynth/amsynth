/* amSynth
 * (c) 2002,2003 Nick Dowell
 */

#ifndef _REQUEST_H
#define _REQUEST_H

#include <sigc++/slot.h>

struct __Request {
  SigC::Slot0<void> slot;
};

typedef struct __Request Request;

#endif
