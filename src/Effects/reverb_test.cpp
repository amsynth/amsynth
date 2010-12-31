
#include "revmodel.hpp"

#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 1024
#define NUM_ITERATIONS 4096

#if __SSE2_MATH__
#include <xmmintrin.h>
#endif

//void disable_denormals()
//{
//#if __SSE2_MATH__
//	int csr = _mm_getcsr();
//	_mm_setcsr(csr | 0x8040);
//#endif
//}

int main(int argc, char *argv[])
{
	int i;
	revmodel reverb;
	
	disable_denormals();

	float *buffer_l = new float[BUFFER_SIZE];
	float *buffer_r = new float[BUFFER_SIZE];

	memset(buffer_l, 0, sizeof(float) * BUFFER_SIZE);
	memset(buffer_r, 0, sizeof(float) * BUFFER_SIZE);

	if (argc > 1) {
		fflush(stdout);
		buffer_l[ 1] = 0.2;
		buffer_l[ 2] = 0.4;
		buffer_l[ 3] = 0.6;
		buffer_l[ 4] = 0.8;
		buffer_l[ 5] = 0.9;
		buffer_l[ 6] = 1.0;
		buffer_l[ 7] = 0.9;
		buffer_l[ 8] = 0.8;
		buffer_l[ 9] = 0.6;
		buffer_l[10] = 0.4;
		buffer_l[11] = 0.2;
		buffer_l[12] = 0.0;
	}

	for (i=0; i<NUM_ITERATIONS; i++) {
		reverb.processreplace(buffer_l, buffer_l, buffer_r, BUFFER_SIZE, 1, 1);
		memset(buffer_l, 0, sizeof(float) * BUFFER_SIZE);
		memset(buffer_r, 0, sizeof(float) * BUFFER_SIZE);
	}
	
	return 0;
}

