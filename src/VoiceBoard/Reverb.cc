/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#include "Reverb.h"

Reverb::Reverb()
{
//	model.mute();
}

void
Reverb::update()
{
	model.setmode( modeParam->getControlValue() );
	model.setroomsize( roomSizeParam->getControlValue() );
	model.setdamp( dampParam->getControlValue() );
	model.setwet( wetParam->getControlValue() );
	model.setdry( dryParam->getControlValue() );
	model.setwidth( widthParam->getControlValue() );
	//model.update();
}

float *
Reverb::getNFData()
{
	inbuffer = input->getFData();
	model.processreplace( inbuffer, inbuffer, outbufferL, outbufferR, BUF_SIZE, 1 );
	// combine the channel buffers into a stereo buffer
	register int ch = 0;
	register int idx = 0;
	for(int i=0; i<BUF_SIZE*2; i++)
		outBuffer[i] = ( (ch=1-ch) ? outbufferL[idx] : outbufferR[idx++] );
	return outBuffer;
}
