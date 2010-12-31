
#include "denormals.h"

#include <float.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define NUM_ITERATIONS 4096

enum
{
	kDenormalFlag  = (1 << 1),
	kUnderflowFlag = (1 << 4),
};

int main(int argc, char *argv[])
{
	disable_denormals();

	float sample = 0.0f;
	if (argc > 1)
		sample = FLT_MIN;
	
#if __SSE2_MATH__
	_mm_setcsr(_mm_getcsr() & ~kDenormalFlag);
#endif

	for (int i=0; i<200; i++) {
		for (int j=0; j<100000; j++) {
			sample *= 0.9f;
			sample *= 1.1f;
			sample *= 0.9f;
			sample *= 1.1f;
			undenormalise(sample);
		}
	}
	
#if __SSE2_MATH__
	if (_mm_getcsr() & kDenormalFlag) {
		_mm_setcsr(_mm_getcsr() & ~kDenormalFlag);
		printf("Denormal processing took place\n");
	}
#endif
	
	return 0;
}

