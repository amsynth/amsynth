/* amSynth
 * (c) 2001,2002 Nick Dowell
 */

#ifndef _ADDER_H
#define _ADDER_H

#include "Synth--.h"
#define MAX_INPUTS 128

/**
 * @class Adder
 * @brief Adds several inputs together.
 */
class Adder: public FSource {
  public:
    Adder();
    virtual ~Adder();
	/**
	 * add a source to be added to the others to find the output.
	 * @param source a source to be added
	 */
    void addInput(FSource & source);
	/**
	 * stop a source being added to find the output.
	 * @param source the source to no longer be used
	 */
    void removeInput(FSource & source);
    inline float *getFData();
  private:
    float *_buffer;
    float * inBuffer;
    FSource *inputs[MAX_INPUTS];
    int inputExists[MAX_INPUTS], no_of_inputs;
};

#endif
