/* amSynth
 * (c) 2001,2002 Nick Dowell
 **/
#include "Mixer.h"

Mixer::Mixer()
{
    buffer = new float[BUF_SIZE];
    mode_param = 0;
    mix_mode = 0;
}

Mixer::~Mixer()
{
    delete[]buffer;
}

void
Mixer::setInput1(FSource & source)
{
    input1 = &source;
}

void 
Mixer::setInput2(FSource & source)
{
    input2 = &source;
}

void 
Mixer::setControl(NFSource & source)
{
    control = &source;
}

void 
Mixer::setMode(Parameter & param)
{
    mode_param = &param;
    mode_param->addUpdateListener(*this);
    update();
}

void 
Mixer::update()
{
    if (mode_param) mix_mode = (int) mode_param->getValue();
} float *
Mixer::getNFData()
{
    inBuffer1 = input1->getFData();
    inBuffer2 = input2->getFData();
	
	register int i;
	if (mix_mode == 0) {
	    controlBuffer = control->getNFData();
		register float mix;
    	for (i = 0; i < BUF_SIZE; i++) {
			mix = (controlBuffer[i] + 1.0) / 2;
			buffer[i] = inBuffer1[i] * (1 - mix) + inBuffer2[i] * mix;
		}
    } else if (mix_mode == 1) {
		for (i = 0; i < BUF_SIZE; i++)
			buffer[i] = inBuffer1[i] * inBuffer2[i];
    }
    return buffer;
}
