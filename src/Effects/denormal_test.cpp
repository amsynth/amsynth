
//
// Note - don't compile with optimisations enabled, currenly that will optimise out all the floating point operations.
//

#define ALWAYS_UNDENORMALISE
#include "denormals.h"

#include <stdio.h>
#include <string.h>
#include <sys/resource.h>

/* returns number of usecs by test run */
unsigned long run_test(bool trigger_denormal, bool kill_denormals)
{
	struct rusage usage_before; 
	getrusage (RUSAGE_SELF, &usage_before);

	float sample = trigger_denormal ? FLT_MIN : 0.0f;

	for (int i=0; i<100; i++) {
		for (int j=0; j<100000; j++) {
			sample *= (0.9f + i / 500.0f);
			sample *= (0.9f + i / 500.0f);
			sample *= (0.9f + i / 500.0f);
			sample *= (0.9f + i / 500.0f);
			if (kill_denormals)
				undenormalise(sample);
		}
	}
	
	struct rusage usage_after; 
	getrusage(RUSAGE_SELF, &usage_after);
	
	unsigned long user_usec = (usage_after.ru_utime.tv_sec*1000000 + usage_after.ru_utime.tv_usec)
							- (usage_before.ru_utime.tv_sec*1000000 + usage_before.ru_utime.tv_usec);
	return user_usec;
}

unsigned long best_test_time(bool trigger_denormal, bool kill_denormal, unsigned num_runs)
{
	unsigned long result = 0;
	for (unsigned i=0; i<num_runs; i++) {
		unsigned long rt = run_test(trigger_denormal, kill_denormal);
		if (result == 0 || result > rt)
			result = rt;
	}
	return result;
}

int main(int argc, char *argv[])
{
	enum
	{
		kDenormalFlag  = (1 << 1),
		kUnderflowFlag = (1 << 4),
	};

#if __SSE2_MATH__
	_mm_setcsr(_mm_getcsr() & ~kDenormalFlag);
#endif
	
	printf("no denormals:                  %.2f\n", best_test_time(false, false, 5) / 1000000.f);
	printf("no denormals, tested for:      %.2f\n", best_test_time(false,  true, 5) / 1000000.f);
	printf("with denormals:                %.2f\n", best_test_time( true, false, 1) / 1000000.f);
	printf("with denormals, tested for:    %.2f\n", best_test_time( true,  true, 1) / 1000000.f);
#if __SSE2_MATH__
	disable_denormals();
	printf("with denormals-as-zero (SSE2): %.2f\n", best_test_time( true, false, 5) / 1000000.f);
#endif
	
//#if __SSE2_MATH__
//	if (_mm_getcsr() & kDenormalFlag) {
//		_mm_setcsr(_mm_getcsr() & ~kDenormalFlag);
//		printf("Denormal processing took place\n");
//	}
//#endif
	
	return 0;
}

