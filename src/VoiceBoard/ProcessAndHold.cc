/* amSynth
 * (c)2001,2002 Nick Dowell
 */

#include "ProcessAndHold.h"

ProcessAndHold::ProcessAndHold()
{
}

ProcessAndHold::~ProcessAndHold()
{
}

void
ProcessAndHold::setInput( FSource & source )
{
	input = &source;
}

void
ProcessAndHold::process()
{
	buffer = input->getFData();}

float *
ProcessAndHold::getFData()
{
	return buffer;
}