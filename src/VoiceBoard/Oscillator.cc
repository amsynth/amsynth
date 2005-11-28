/* amSynth
 * (c) 2001-2005 Nick Dowell
 **/

#include <cmath>
#include <cstdlib>		// required for rand()
#include <iostream>
#include "Oscillator.h"

// fmod is sloooooooooooow.
#undef fmodf
#undef fmodf
inline float fmodf (float x, float y) {
	while (x > y) x -= y;
	return x;
}

Oscillator::Oscillator()
:	rads (0.0)
,	random (0)
,	waveform (Waveform_Sine)
,	rate (44100)
,	random_count (0)
,	period (0)
,	reset_offset (4096)
,	reset_period (4096)
,	sync (NULL)
{}

void Oscillator::SetWaveform	(Waveform w)			{ waveform = w; }
void Oscillator::reset			()						{ rads = 0.0; }
void Oscillator::reset			(int offset, int period){ reset_offset = offset; reset_period = period; }

void Oscillator::SetSync		(Oscillator* o)
{
	if (sync) sync->reset (4096, 4096);
	sync = o;
	reset_period = 4096;
	reset_offset = 4096;
}

void
Oscillator::ProcessSamples	(float *buffer, int numSamples, float freq_hz, float pw)
{
	freq = freq_hz;
	mPulseWidth = pw;
	
	sync_c = 0;
	sync_offset = 65;
		
	reset_cd = reset_offset;
	
	switch (waveform)
	{
	case Waveform_Sine:		doSine		(buffer, numSamples);	break;
	case Waveform_Pulse:	doSquare	(buffer, numSamples);	break;
	case Waveform_Saw:		doSaw		(buffer, numSamples);	break;
	case Waveform_Noise:	doNoise		(buffer, numSamples);	break;
	case Waveform_Random:	doRandom	(buffer, numSamples);	break;
	default: break;
	}
	
	if (sync) sync->reset (sync_offset, (int)(rate/freq));
}

void 
Oscillator::doSine(float *buffer, int nFrames)
{
    for (int i = 0; i < nFrames; i++) {
		buffer[i] = sinf(rads += twopi_rate * freq);
		//-- sync to other oscillator --
		if (reset_cd-- == 0){
			rads = 0.0;					// reset the oscillator
			reset_cd = reset_period-1;	// start counting down again
		}
		if ( sync_offset > nFrames)	// then we havent already found the offset
			if( rads > TWO_PI )			// then weve completed a circle
				sync_offset = i;		// remember the offset
	}
	rads = fmodf((float)rads, (float)TWO_PI);			// overflows are bad!
}

float 
Oscillator::sqr(float foo)
{
    if ((fmodf((float)foo, (float)TWO_PI)) < (mPulseWidth + 1) * PI)
	return 1.0;
    else
	return -1.0;
}

void 
Oscillator::doSquare(float *buffer, int nFrames)
{
    for (int i = 0; i < nFrames; i++) {
		buffer[i] = sqr(rads += (twopi_rate * freq));
		//-- sync to other oscillator --
		if (reset_cd-- == 0){
			rads = 0.0;					// reset the oscillator
			reset_cd = reset_period-1;	// start counting down again
		}
		if ( sync_offset > nFrames)	// then we havent already found the offset
			if( rads > TWO_PI )			// then weve completed a circle
				sync_offset = i;		// remember the offset
	}
    rads = fmodf((float)rads, (float)TWO_PI);
}


float 
Oscillator::saw(float foo)
{
    foo = fmodf((float)foo, (float)TWO_PI);
    register float t = (float) (foo / (2 * PI));
    register float a = (mPulseWidth + 1) / 2;
    if (t < a / 2)
	return 2 * t / a;
    else if (t > (1 - (a / 2)))
	return (2 * t - 2) / a;
    else
	return (1 - 2 * t) / (1 - a);
//    return 1.0 - (fmodf((float)foo, (float)TWO_PI) / PI);
}

void 
Oscillator::doSaw(float *buffer, int nFrames)
{
    for (int i = 0; i < nFrames; i++) {
		buffer[i] = saw(rads += (twopi_rate * freq));
		//-- sync to other oscillator --
		if (reset_cd-- == 0){
			rads = 0.0;					// reset the oscillator
			reset_cd = reset_period-1;	// start counting down again
		}
		if ( sync_offset > nFrames)	// then we havent already found the offset
			if( rads > TWO_PI )			// then weve completed a circle
				sync_offset = i;		// remember the offset
	}
    rads = fmodf((float)rads, (float)TWO_PI);
}

void 
Oscillator::doRandom(float *buffer, int nFrames)
{
    register int period = (int) (rate / freq);
    for (int i = 0; i < nFrames; i++) {
	if (random_count > period) {
	    random_count = 0;
		random = ((float)::rand() / (RAND_MAX / 2.0f)) - 1.0f;
	}
	random_count++;
	buffer[i] = random;
    }
}

void 
Oscillator::doNoise(float *buffer, int nFrames)
{
    for (int i = 0; i < nFrames; i++)
		buffer[i] = ((float)::rand() / (RAND_MAX / 2.0f)) - 1.0f;
}
