/* amSynth
 * (c) 2001,2002 Nick Dowell
 **/
#include "Amplifier.h"
#include <iostream>

Amplifier::Amplifier(float *buf)
{
    buffer = buf;
}

Amplifier::~Amplifier()
{
    delete[]buffer;
}

void
 Amplifier::setInput(NFSource & source)
{
    source1 = &source;
}

void Amplifier::setCInput(NFSource & source)
{
    source2 = &source;
}

float *Amplifier::getNFData()
{
    buffer1 = source1->getNFData();
    buffer2 = source2->getNFData();

    for (int i = 0; i < BUF_SIZE; i++)
		buffer[i] = (buffer1[i] * buffer2[i]);
    return buffer;
}
