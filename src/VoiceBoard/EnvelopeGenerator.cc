/* amSynth
 * (c) 2001,2002 Nick Dowell
 **/
#include "EnvelopeGenerator.h"

EnvelopeGenerator::EnvelopeGenerator()
{
    value = 0.0;
    buffer = new float[BUF_SIZE];
}

EnvelopeGenerator::~EnvelopeGenerator()
{
    delete[]buffer;
}

float *EnvelopeGenerator::getNFData()
{
	for (int i = 0; i < BUF_SIZE; i++) buffer[i] = value;
  return buffer;
}

void EnvelopeGenerator::triggerOn()
{
    value = 1.0;
}

void EnvelopeGenerator::triggerOff()
{
    value = 0.0;
}
