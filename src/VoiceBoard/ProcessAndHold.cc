/* amSynth
 * (c)2001,2002 Nick Dowell
 */

#include "ProcessAndHold.h"

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
