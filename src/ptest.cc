/* amSynth
 * (c) 2001-2005 Nick Dowell
 */

#include "VoiceAllocationUnit.h"

#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>

int main ()
{
	//
	// test parameters
	// 
	const int kTestBufSize = 256;
	const int kTestSampleRate = 44100;
	const int kTimeSeconds = 60;
	const int kNumVoices = 10;

	float *buffer = new float [kTestBufSize];

	VoiceAllocationUnit *vau = new VoiceAllocationUnit;
	vau->SetSampleRate (kTestSampleRate);
	
	// trigger off some notes for amSynth to render.
	for (int v=0; v<kNumVoices; v++) vau->noteOn (60+v, 127);
	
	struct rusage usage_before; 
	getrusage (RUSAGE_SELF, &usage_before);
	
	long total_samples = kTestSampleRate * kTimeSeconds;
	long total_calls = total_samples / kTestBufSize;
	long remain_samples = total_samples % kTestBufSize;
	for (int i=0; i<total_calls; i++) vau->Process (buffer, buffer, kTestBufSize);
	vau->Process (buffer, buffer, remain_samples);

	struct rusage usage_after; 
	getrusage (RUSAGE_SELF, &usage_after);
	
	unsigned long user_usec = (usage_after.ru_utime.tv_sec*1000000 + usage_after.ru_utime.tv_usec)
							- (usage_before.ru_utime.tv_sec*1000000 + usage_before.ru_utime.tv_usec);
	
	unsigned long syst_usec = (usage_after.ru_stime.tv_sec*1000000 + usage_after.ru_stime.tv_usec)
							- (usage_before.ru_stime.tv_sec*1000000 + usage_before.ru_stime.tv_usec);

	unsigned long usec_audio = kTimeSeconds * kNumVoices * 1000000;
	unsigned long usec_cpu = user_usec + syst_usec;
	
	fprintf (stderr, "user time: %f		system time: %f\n", user_usec/1000000.f, syst_usec/1000000.f);
	fprintf (stderr, "performance index: %f\n", (float) usec_audio / (float) usec_cpu);
	
	delete [] buffer;
	delete vau;
	return 0;
}
