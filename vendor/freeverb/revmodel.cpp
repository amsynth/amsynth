// Reverb model implementation
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#include <assert.h>
#include <iostream>
#include "revmodel.hpp"

revmodel::revmodel()
:	mode(initialmode)
,	dryz(initialdry)
,	wet1z(0.F)
,	wet2z(0.F)
{
    setrate(44100);

	// Set default values
	allpassL[0].setfeedback(0.5f);
	allpassR[0].setfeedback(0.5f);
	allpassL[1].setfeedback(0.5f);
	allpassR[1].setfeedback(0.5f);
	allpassL[2].setfeedback(0.5f);
	allpassR[2].setfeedback(0.5f);
	allpassL[3].setfeedback(0.5f);
	allpassR[3].setfeedback(0.5f);
	setwet(initialwet);
	setroomsize(initialroom);
	setdry(initialdry);
	setdamp(initialdamp);
	setwidth(initialwidth);
	setmode(initialmode);

	update();
}

void revmodel::setrate(int rate)
{
    assert(rate <= TUNING_MAX_SAMPLE_RATE);

    combL[0].setbuffer(bufcombL1, TUNING(combtuningL1, rate));
    combR[0].setbuffer(bufcombR1, TUNING(combtuningR1, rate));
    combL[1].setbuffer(bufcombL2, TUNING(combtuningL2, rate));
    combR[1].setbuffer(bufcombR2, TUNING(combtuningR2, rate));
    combL[2].setbuffer(bufcombL3, TUNING(combtuningL3, rate));
    combR[2].setbuffer(bufcombR3, TUNING(combtuningR3, rate));
    combL[3].setbuffer(bufcombL4, TUNING(combtuningL4, rate));
    combR[3].setbuffer(bufcombR4, TUNING(combtuningR4, rate));
    combL[4].setbuffer(bufcombL5, TUNING(combtuningL5, rate));
    combR[4].setbuffer(bufcombR5, TUNING(combtuningR5, rate));
    combL[5].setbuffer(bufcombL6, TUNING(combtuningL6, rate));
    combR[5].setbuffer(bufcombR6, TUNING(combtuningR6, rate));
    combL[6].setbuffer(bufcombL7, TUNING(combtuningL7, rate));
    combR[6].setbuffer(bufcombR7, TUNING(combtuningR7, rate));
    combL[7].setbuffer(bufcombL8, TUNING(combtuningL8, rate));
    combR[7].setbuffer(bufcombR8, TUNING(combtuningR8, rate));
    allpassL[0].setbuffer(bufallpassL1, TUNING(allpasstuningL1, rate));
    allpassR[0].setbuffer(bufallpassR1, TUNING(allpasstuningR1, rate));
    allpassL[1].setbuffer(bufallpassL2, TUNING(allpasstuningL2, rate));
    allpassR[1].setbuffer(bufallpassR2, TUNING(allpasstuningR2, rate));
    allpassL[2].setbuffer(bufallpassL3, TUNING(allpasstuningL3, rate));
    allpassR[2].setbuffer(bufallpassR3, TUNING(allpasstuningR3, rate));
    allpassL[3].setbuffer(bufallpassL4, TUNING(allpasstuningL4, rate));
    allpassR[3].setbuffer(bufallpassR4, TUNING(allpasstuningR4, rate));

    // Buffer will be full of rubbish - so we MUST mute them
    mute();
}

void revmodel::mute()
{
	if (getmode() >= freezemode)
		return;

	for (int i=0;i<numcombs;i++)
	{
		combL[i].mute();
		combR[i].mute();
	}
	for (int i=0;i<numallpasses;i++)
	{
		allpassL[i].mute();
		allpassR[i].mute();
	}
}

void 
revmodel::processreplace(float *inputL, float *inputR, float *outputL, float *outputR, long numsamples, int skip)
{
	float outL,outR,input;
	int i;

	while(numsamples-- > 0)
	{
		outL = outR = 0;
		input = (*inputL) * gain;

		// Accumulate comb filters in parallel
		for(i=0; i<numcombs; i++)
		{
			outL += combL[i].process(input);
			outR += combR[i].process(input);
		}

		// Feed through allpasses in series
		for(i=0; i<numallpasses; i++)
		{
			outL = allpassL[i].process(outL);
			outR = allpassR[i].process(outR);
		}

		// De-zipper
		float d = (dryz += ((dry - dryz) * 0.005F));
		float w1 = (wet1z += ((wet1 - wet1z) * 0.005F));
		float w2 = (wet2z += ((wet2 - wet2z) * 0.005F));

		// Calculate output REPLACING anything already there
		*outputL = outL*w1 + outR*w2 + *inputL*d;
		*outputR = outR*w1 + outL*w2 + *inputR*d;

		// Increment sample pointers, allowing for interleave (if any)
		inputL += skip;
		inputR += skip;
		outputL += skip;
		outputR += skip;
	}
}

void 
revmodel::processreplace(float *inputM, float *outputL, float *outputR, long numsamples, int stride_in, int stride_out)
{
	float outL,outR,input;
	int i;

	while(numsamples-- > 0)
	{
		outL = outR = 0;
		input = (*inputM) * gain;

		// Accumulate comb filters in parallel
		for(i=0; i<numcombs; i++)
		{
			outL += combL[i].process(input);
			outR += combR[i].process(input);
		}

		// Feed through allpasses in series
		for(i=0; i<numallpasses; i++)
		{
			outL = allpassL[i].process(outL);
			outR = allpassR[i].process(outR);
		}

		// De-zipper
		float d = (dryz += ((dry - dryz) * 0.005F));
		float w1 = (wet1z += ((wet1 - wet1z) * 0.005F));
		float w2 = (wet2z += ((wet2 - wet2z) * 0.005F));

		// Calculate output REPLACING anything already there
		*outputL = outL*w1 + outR*w2 + *inputM*d;
		*outputR = outR*w1 + outL*w2 + *inputM*d;

		// Increment sample pointers, allowing for interleave (if any)
		inputM += stride_in;
		outputL += stride_out;
		outputR += stride_out;
	}
}

void revmodel::processmix(float *inputL, float *inputR, float *outputL, float *outputR, long numsamples, int skip)
{
	float outL,outR,input;

	while(numsamples-- > 0)
	{
		outL = outR = 0;
		input = (*inputL + *inputR) * gain;

		// Accumulate comb filters in parallel
		for(int i=0; i<numcombs; i++)
		{
			outL += combL[i].process(input);
			outR += combR[i].process(input);
		}

		// Feed through allpasses in series
		for(int i=0; i<numallpasses; i++)
		{
			outL = allpassL[i].process(outL);
			outR = allpassR[i].process(outR);
		}

		// De-zipper
		float d = (dryz += ((dry - dryz) * 0.005F));
		float w1 = (wet1z += ((wet1 - wet1z) * 0.005F));
		float w2 = (wet2z += ((wet2 - wet2z) * 0.005F));

		// Calculate output MIXING with anything already there
		*outputL += outL*w1 + outR*w2 + *inputL*d;
		*outputR += outR*w1 + outL*w2 + *inputR*d;

		// Increment sample pointers, allowing for interleave (if any)
		inputL += skip;
		inputR += skip;
		outputL += skip;
		outputR += skip;
	}
}

void revmodel::update()
{
// Recalculate internal values after parameter change

	int i;

	wet1 = wet*(width/2 + 0.5f);
	wet2 = wet*((1-width)/2);

	if (mode >= freezemode)
	{
		roomsize1 = 1;
		damp1 = 0;
		gain = muted;
	}
	else
	{
		roomsize1 = roomsize;
		damp1 = damp;
		gain = fixedgain;
	}

	for(i=0; i<numcombs; i++)
	{
		combL[i].setfeedback(roomsize1);
		combR[i].setfeedback(roomsize1);
	}

	for(i=0; i<numcombs; i++)
	{
		combL[i].setdamp(damp1);
		combR[i].setdamp(damp1);
	}
}

// The following get/set functions are not inlined, because
// speed is never an issue when calling them, and also
// because as you develop the reverb model, you may
// wish to take dynamic action when they are called.

void revmodel::setroomsize(float value)
{
	roomsize = (value*scaleroom) + offsetroom;
	update();
}

float revmodel::getroomsize()
{
	return (roomsize-offsetroom)/scaleroom;
}

void revmodel::setdamp(float value)
{
	damp = value*scaledamp;
	update();
}

float revmodel::getdamp()
{
	return damp/scaledamp;
}

void revmodel::setwet(float value)
{
	wet = value*scalewet;
	update();
}

float revmodel::getwet()
{
	return wet/scalewet;
}

void revmodel::setdry(float value)
{
	dry = value*scaledry;
}

float revmodel::getdry()
{
	return dry/scaledry;
}

void revmodel::setwidth(float value)
{
	width = value;
	update();
}

float revmodel::getwidth()
{
	return width;
}

void revmodel::setmode(float value)
{
	mode = value;
	update();
}

float revmodel::getmode()
{
	if (mode >= freezemode)
		return 1;
	else
		return 0;
}

//ends
