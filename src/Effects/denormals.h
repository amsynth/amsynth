#ifndef _denormals_
#define _denormals_

#include <float.h>

#ifdef __SSE2_MATH__
#include <xmmintrin.h>
#endif

//
// The preferred way to prevent denormal processing and its associated performance penalty is to
// configure the compiler to generate SSE2 instructions for all floating point operations, and
// disable denormal processing by setting the 'Denormals Are Zero' and 'Flush to Zero' (DAZ and FZ)
// bits in the MXCSR. Note that the DAZ flag was assed in SSE2.
//
// -mfpmath=sse (This is the default choice for the x86-64 compiler)
//
// SSE is not available on all processors, though, so cannot be reiled upon.
//

#if __SSE2_MATH__ && !defined(ALWAYS_UNDENORMALISE)
// assuming disable_denormals() was called, denormals will not occur
#define undenormalise(s)
#else
#define undenormalise(s) if ((s) < FLT_MIN) { (s) = 0.0f; }
#endif

static inline void disable_denormals()
{
#if __SSE2_MATH__
	_mm_setcsr(_mm_getcsr() | 0x8040);
#endif
}

#endif//_denormals_

//ends
