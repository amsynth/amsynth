/* amSynth
 * (c) 2001-2004 Nick Dowell
 **/

#include <math.h>
#include <stdlib.h>		// required for random()
#include "Oscillator.h"

Oscillator::Oscillator(int rate, float *buf)
:	outBuffer (buf)
,	rads (0.0)
,	random (0)
,	waveform (Waveform_Sine)
,	rate (rate)
,	random_count (0)
,	period (0)
,	sync (0)
,	syncOsc (0)
{
    twopi_rate = (float) TWO_PI / rate;
    sync = period = 0;
}

void
Oscillator::SetWaveform	(Waveform w)
{
	waveform = w;
	update ();
}

void 
Oscillator::SetSyncOsc	(Oscillator & osc)
{
	syncOsc = &osc;
	update ();
}

void
Oscillator::SetSync	(int value)
{
	sync = value;
	update ();
}

void 
Oscillator::reset()
{
    rads = 0.0;
}

void 
Oscillator::reset(int offset, int period)
{
	reset_offset = offset;
	reset_period = period;
}

void 
Oscillator::update()
{
	if (sync == 0)
	{
		reset_period = 4096;
		reset_offset = 4096;
		if (syncOsc) syncOsc->reset( 4096, 4096 );
	}
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
	
	if (sync) syncOsc->reset (sync_offset, (int)(rate/freq));
}

void 
Oscillator::doSine(float *buffer, int nFrames)
{
    for (int i = 0; i < nFrames; i++) {
		buffer[i] = sin(rads += (twopi_rate * freq));
		//-- sync to other oscillator --
		if (reset_cd-- == 0){
			rads = 0.0;					// reset the oscillator
			reset_cd = reset_period-1;	// start counting down again
		}
		if ( sync_offset > nFrames)	// then we havent already found the offset
			if( rads > TWO_PI )			// then weve completed a circle
				sync_offset = i;		// remember the offset
	}
	rads = fmod((float)rads, (float)TWO_PI);			// overflows are bad!
}

float 
Oscillator::sqr(float foo)
{
    if ((fmod((float)foo, (float)TWO_PI)) < (mPulseWidth + 1) * PI)
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
    rads = fmod((float)rads, (float)TWO_PI);
}


float 
Oscillator::saw(float foo)
{
    foo = fmod((float)foo, (float)TWO_PI);
    register float t = (float) (foo / (2 * PI));
    register float a = (mPulseWidth + 1) / 2;
    if (t < a / 2)
	return 2 * t / a;
    else if (t > (1 - (a / 2)))
	return (2 * t - 2) / a;
    else
	return (1 - 2 * t) / (1 - a);
//    return 1.0 - (fmod((float)foo, (float)TWO_PI) / PI);
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
    rads = fmod((float)rads, (float)TWO_PI);
}

void 
Oscillator::doRandom(float *buffer, int nFrames)
{
    register int period = (int) (rate / freq);
    for (int i = 0; i < nFrames; i++) {
	if (random_count > period) {
	    random_count = 0;
		random = 
#ifdef _WINDOWS
			0.0;
#else
			((float)::random() / (RAND_MAX / 2)) - 1.0;
#endif
	}
	random_count++;
	buffer[i] = random;
    }
}

void 
Oscillator::doNoise(float *buffer, int nFrames)
{
    for (int i = 0; i < nFrames; i++)
		buffer[i] = 
#ifdef _WINDOWS
		0;
#else
		((float)::random() / (RAND_MAX / 2)) - 1.0;
#endif
}
