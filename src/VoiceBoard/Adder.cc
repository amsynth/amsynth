/* amSynth
 * (c) 2001 Nick Dowell
 */
#include "Adder.h"
#ifdef _DEBUG
#include <iostream>
#endif
Adder::Adder()
{
	no_of_inputs = 0;
	_buffer = new float[BUF_SIZE];
	inBuffer = new float[BUF_SIZE];
	for (int i = 0; i < MAX_INPUTS; i++)
		inputExists[i] = 0;
}

Adder::~Adder()
{
    delete[]_buffer;
}

void
Adder::addInput(FSource & source)
{
#ifdef _DEBUG
    cout << "<Adder> addInput()" << endl;
#endif
    no_of_inputs++;

#ifdef _DEBUG
    for (int i = 0; i < MAX_INPUTS; i++)
	if (&source == inputs[i])
	    cout << "<Adder> Youre adding the same source again..." <<
		endl;
#endif
    //iterate through all the inputExists[] to find a free slot, then add..
    for (int i = 0; i < MAX_INPUTS; i++) {
	if (!inputExists[i]) {
//                      _gain = 1/(float)no_of_inputs;
	    inputExists[i] = 1;
	    inputs[i] = &source;
#ifdef _DEBUG
	    cout << "<Adder> added input at index " << i << endl;
#endif
	    return;
	}
    }
#ifdef _DEBUG
    cout << "<Adder> addInput() failed" << endl;
#endif
}

void
Adder::removeInput(FSource & source)
{
#ifdef _DEBUG
    cout << "<Adder> removeInput()" << endl;
#endif
    for (int i = 0; i < MAX_INPUTS; i++)
	if (inputExists[i] && &source == inputs[i]) {
#ifdef _DEBUG
	    cout << "<Adder> found input to remove at index " << i << endl;
#endif
	    inputExists[i] = 0;
	    no_of_inputs--;
	}
}

float *
Adder::getFData()
{
	for(int i=0; i<BUF_SIZE; i++) _buffer[i] = 0;

  //get all audio data and accumulate all the input buffers
  for (int input = 0; input < MAX_INPUTS; input++) {
		if (inputExists[input]) {
	   	inBuffer = inputs[input]->getFData();
	   	for (int i = 0; i < BUF_SIZE; i++)
			_buffer[i] += inBuffer[i];
		}
	}
  return _buffer;
}
