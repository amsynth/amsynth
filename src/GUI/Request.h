/* amSynth
 * (c) 2002-2005 Nick Dowell
 */

#ifndef _REQUEST_H
#define _REQUEST_H

#include <sigc++/slot.h>

struct __Request {
  sigc::slot<void> slot;
};

typedef struct __Request Request;

#endif
