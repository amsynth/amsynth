#ifndef _denormals_
#define _denormals_

#include <float.h>

#define undenormalise(s) if ((s) < FLT_MIN) { (s) = 0.0f; }

#endif//_denormals_
