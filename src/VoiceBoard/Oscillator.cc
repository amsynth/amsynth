/* amSynth
 * (c) 2001,2002 Nick Dowell
 **/

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include "Oscillator.h"

Oscillator::Oscillator(int rate, float *buf)
{
    outBuffer = buf;
    pulseBuffer = new float[1];
    pulseWidth = 0;
    rads = 0.0;
    waveform = 1;
    waveParam = 0;
    random = 0;
    random_count = 0;
    this->rate = rate;
    twopi_rate = TWO_PI / rate;
    syncParam = 0;
    sync = period = 0;
}

Oscillator::~Oscillator()
{
    delete[] pulseBuffer;
}

void
Oscillator::setInput(FSource & source)
{
    input = &source;
}

void 
Oscillator::setWaveform(Parameter & param)
{
    waveParam = &param;
    param.addUpdateListener(*this);
    update();
}

void 
Oscillator::setSync(Parameter & param, Oscillator & osc)
{
    syncParam = &param;
    syncOsc = &osc;
    param.addUpdateListener(*this);
    update();
}

void 
Oscillator::setPulseWidth(FSource & source)
{
    pulseWidth = &source;
    delete[] pulseBuffer;
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
    if (waveParam)
		waveform = (int) waveParam->getValue();
	sync = 0;
    if (syncParam)
		sync = (int) syncParam->getValue();
	if(sync==0){
		reset_period = BUF_SIZE+1;
		reset_offset = BUF_SIZE+1;
		if(syncOsc)
			syncOsc->reset( BUF_SIZE+1, BUF_SIZE+1 );
	}
}

float *
Oscillator::getNFData()
{
    // do we really need to track the frequency _every_ sample??
    inBuffer = input->getFData();
    freq = inBuffer[0];
    
	sync_c = 0;
    sync_offset = BUF_SIZE + 1;
	
    reset_cd = reset_offset;
    
	if (pulseWidth) {
		pulseBuffer = pulseWidth->getFData();
    } else {
		pulseBuffer[0] = 0;
    }

    // any decision statements are BAD in real-time code...
    switch (waveform) {
	case 0:
		doSine();
		break;
	case 1:
		doSquare();
		break;
	case 2:
		doSaw();
		break;
	case 3:
		doNoise();
		break;
	case 4:
		doRandom();
		break;
	default:
		break;
	}
	if(sync)
		syncOsc->reset( sync_offset, (int)(rate/freq) );
    return outBuffer;
}

void 
Oscillator::doSine()
{
    for (int i = 0; i < BUF_SIZE; i++) {
		outBuffer[i] = sin(rads += (twopi_rate * freq));
		//-- sync to other oscillator --
		if (reset_cd-- == 0){
			rads = 0.0;					// reset the oscillator
			reset_cd = reset_period-1;	// start counting down again
		}
		if ( sync_offset > BUF_SIZE)	// then we havent already found the offset
			if( rads > TWO_PI )			// then weve completed a circle
				sync_offset = i;		// remember the offset
	}
	rads = fmod((float)rads, (float)TWO_PI);			// overflows are bad!
}

float 
Oscillator::sqr(float foo)
{
    if ((fmod((float)foo, (float)TWO_PI)) < (pulseBuffer[0] + 1) * PI)
	return 1.0;
    else
	return -1.0;
}

void 
Oscillator::doSquare()
{
    for (int i = 0; i < BUF_SIZE; i++) {
		outBuffer[i] = sqr(rads += (twopi_rate * freq));
		//-- sync to other oscillator --
		if (reset_cd-- == 0){
			rads = 0.0;					// reset the oscillator
			reset_cd = reset_period-1;	// start counting down again
		}
		if ( sync_offset > BUF_SIZE)	// then we havent already found the offset
			if( rads > TWO_PI )			// then weve completed a circle
				sync_offset = i;		// remember the offset
	}
    rads = fmod((float)rads, (float)TWO_PI);
}


float 
Oscillator::saw(float foo)
{
    foo = fmod((float)foo, (float)TWO_PI);
    register float t = (foo / (2 * PI));
    register float a = (pulseBuffer[0] + 1) / 2;
    if (t < a / 2)
	return 2 * t / a;
    else if (t > (1 - (a / 2)))
	return (2 * t - 2) / a;
    else
	return (1 - 2 * t) / (1 - a);
//    return 1.0 - (fmod((float)foo, (float)TWO_PI) / PI);
}

void 
Oscillator::doSaw()
{
    for (int i = 0; i < BUF_SIZE; i++) {
		outBuffer[i] = saw(rads += (twopi_rate * freq));
		//-- sync to other oscillator --
		if (reset_cd-- == 0){
			rads = 0.0;					// reset the oscillator
			reset_cd = reset_period-1;	// start counting down again
		}
		if ( sync_offset > BUF_SIZE)	// then we havent already found the offset
			if( rads > TWO_PI )			// then weve completed a circle
				sync_offset = i;		// remember the offset
	}
    rads = fmod((float)rads, (float)TWO_PI);
}

void 
Oscillator::doRandom()
{
    register int period = (int) (rate / inBuffer[0]);
    for (int i = 0; i < BUF_SIZE; i++) {
	if (random_count > period) {
	    random_count = 0;
	    random = ((float)::random() / (RAND_MAX / 2)) - 1.0;
	}
	random_count++;
	outBuffer[i] = random;
    }
}

void 
Oscillator::doNoise()
{
    for (int i = 0; i < BUF_SIZE; i++)
	outBuffer[i] = ((float)::random() / (RAND_MAX / 2)) - 1.0;
}
