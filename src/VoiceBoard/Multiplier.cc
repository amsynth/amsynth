 /* amSynth
 * (c) 2001,2002 Nick Dowell
 **/
#include "Multiplier.h"
#ifdef _DEBUG
#include <iostream>
#endif
Multiplier::Multiplier()
{
	no_of_inputs = 0;
	_buffer = new float[BUF_SIZE];
	for (int i=0; i<BUF_SIZE; i++) _buffer[i] = 0;
	for (int i = 0; i < MAX_INPUTS; i++) inputExists[i] = 0;
}

Multiplier::~Multiplier()
{
	delete[]_buffer;
}

void Multiplier::addInput(FSource & source)
{
#ifdef _DEBUG
  cout << "<Multiplier> addInput()" << endl;
#endif
  no_of_inputs++;

#ifdef _DEBUG
  for (int i = 0; i < MAX_INPUTS; i++)
    if (&source == inputs[i])
      cout << "<Multiplier> Youre adding the same source again..." << endl;
#endif
  //iterate through all the inputExists[] to find a free slot, then add..
  for (int i = 0; i < MAX_INPUTS; i++) {
    if (!inputExists[i]) {
      inputExists[i] = 1;
      inputs[i] = &source;
#ifdef _DEBUG
      cout << "<Multiplier> added input at index " << i << endl;
#endif
      return;
    }
  }
#ifdef _DEBUG
  cout << "<Multiplier> addInput() failed" << endl;
#endif
}

void
Multiplier::removeInput(FSource & source)
{
#ifdef _DEBUG
  cout << "<Multiplier> removeInput()" << endl;
#endif
  for (int i = 0; i < MAX_INPUTS; i++)
    if (inputExists[i] && &source == inputs[i]) {
#ifdef _DEBUG
      cout << "<Multiplier> found input to remove at index " << i << endl;
#endif
      inputExists[i] = 0;
      no_of_inputs--;
    }
}

float *
Multiplier::getFData()
{
  for (int i=0; i<BUF_SIZE; i++) _buffer[i]=1;
  
  for (int input = 0; input < MAX_INPUTS; input++) {
    if (inputExists[input]) {
      inBuffer = inputs[input]->getFData();
      for (int i = 0; i < BUF_SIZE; i++)
	_buffer[i] *= inBuffer[i];
    }
  }
  return _buffer;
}
