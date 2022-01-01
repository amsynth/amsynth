// Reverb model declaration
//
// Written by Jezar at Dreampoint, June 2000
// http://www.dreampoint.co.uk
// This code is public domain

#ifndef _revmodel_
#define _revmodel_

#include "comb.hpp"
#include "allpass.hpp"
#include "tuning.h"

class revmodel
{
public:
	revmodel();
    void    setrate(int rate);
    void    mute();
    void    processmix(float *inputL, float *inputR, float *outputL, float *outputR, long numsamples, int skip);
    void    processreplace(float *inputL, float *inputR, float *outputL, float *outputR, long numsamples, int skip);
    void    processreplace(float *inputM, float *outputL, float *outputR, long numsamples, int stride_in, int stride_out);
    void    setroomsize(float value);
    float   getroomsize();
    void    setdamp(float value);
    float   getdamp();
    void    setwet(float value);
    float   getwet();
    void    setdry(float value);
    float   getdry();
    void    setwidth(float value);
    float   getwidth();
    void    setmode(float value);
    float   getmode();
private:
	void    update();
private:
    float   gain;
	float   roomsize,roomsize1;
    float   damp,damp1;
    float   wet;
    float   dry, wet1, wet2;
    float   dryz, wet1z, wet2z;
   float   width;
 float   mode;

 // The following are all declared inline 
      // to remove the need for dynamic allocation
   // with its subsequent error-checking messiness

       // Comb filters
    comb    combL[numcombs];
    comb    combR[numcombs];

      // Allpass filters
    allpass allpassL[numallpasses];
    allpass allpassR[numallpasses];

    // Buffers for the combs
    float   bufcombL1[TUNING(combtuningL1, TUNING_MAX_SAMPLE_RATE)];
    float   bufcombR1[TUNING(combtuningR1, TUNING_MAX_SAMPLE_RATE)];
    float   bufcombL2[TUNING(combtuningL2, TUNING_MAX_SAMPLE_RATE)];
    float   bufcombR2[TUNING(combtuningR2, TUNING_MAX_SAMPLE_RATE)];
    float   bufcombL3[TUNING(combtuningL3, TUNING_MAX_SAMPLE_RATE)];
    float   bufcombR3[TUNING(combtuningR3, TUNING_MAX_SAMPLE_RATE)];
    float   bufcombL4[TUNING(combtuningL4, TUNING_MAX_SAMPLE_RATE)];
    float   bufcombR4[TUNING(combtuningR4, TUNING_MAX_SAMPLE_RATE)];
    float   bufcombL5[TUNING(combtuningL5, TUNING_MAX_SAMPLE_RATE)];
    float   bufcombR5[TUNING(combtuningR5, TUNING_MAX_SAMPLE_RATE)];
    float   bufcombL6[TUNING(combtuningL6, TUNING_MAX_SAMPLE_RATE)];
    float   bufcombR6[TUNING(combtuningR6, TUNING_MAX_SAMPLE_RATE)];
    float   bufcombL7[TUNING(combtuningL7, TUNING_MAX_SAMPLE_RATE)];
    float   bufcombR7[TUNING(combtuningR7, TUNING_MAX_SAMPLE_RATE)];
    float   bufcombL8[TUNING(combtuningL8, TUNING_MAX_SAMPLE_RATE)];
    float   bufcombR8[TUNING(combtuningR8, TUNING_MAX_SAMPLE_RATE)];

    // Buffers for the allpasses
    float   bufallpassL1[TUNING(allpasstuningL1, TUNING_MAX_SAMPLE_RATE)];
    float   bufallpassR1[TUNING(allpasstuningR1, TUNING_MAX_SAMPLE_RATE)];
    float   bufallpassL2[TUNING(allpasstuningL2, TUNING_MAX_SAMPLE_RATE)];
    float   bufallpassR2[TUNING(allpasstuningR2, TUNING_MAX_SAMPLE_RATE)];
    float   bufallpassL3[TUNING(allpasstuningL3, TUNING_MAX_SAMPLE_RATE)];
    float   bufallpassR3[TUNING(allpasstuningR3, TUNING_MAX_SAMPLE_RATE)];
    float   bufallpassL4[TUNING(allpasstuningL4, TUNING_MAX_SAMPLE_RATE)];
    float   bufallpassR4[TUNING(allpasstuningR4, TUNING_MAX_SAMPLE_RATE)];
};

#endif//_revmodel_

//ends
