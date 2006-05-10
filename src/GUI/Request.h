/* amSynth
 * (c) 2002-2006 Nick Dowell
 */

#ifndef _REQUEST_H
#define _REQUEST_H

#include <sigc++/slot.h>

typedef struct {
  sigc::slot<void> slot;
} Request;

#endif
