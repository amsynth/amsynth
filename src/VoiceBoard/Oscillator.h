/* amSynth
 * (c) 2001,2002 Nick Dowell
 */
#ifndef _OSCILLATOR_H
#define _OSCILLATOR_H

#include "Synth--.h"
#include "../Parameter.h"

class Oscillator:public NFSource, public FInput, public UpdateListener {
  public:
    Oscillator(int rate);
    virtual ~Oscillator();
    inline float *getNFData();
    void setInput(FSource & source);
	void setPulseWidth(FSource & source);
    void setWaveform(int form);
    void setWaveform(Parameter & param);
	/* resets the oscillator.
	 * i.e. from the next call the oscillator will begin its cycle from the
	 * beginning.
	 */
	inline void reset();
	/*
	 * reset the oscillator, initially at sample indicated by offset, and then every
	 * period samples. used for oscillator sync. 
	 * NB. period >= delta
	 */
    inline void reset( int offset, int period );
    void update();
	void setSync( Parameter & param, Oscillator & osc );
  private:
    FSource *input;
	FSource *pulseWidth;
    float *inBuffer, *outBuffer, *pulseBuffer;
    float rads, twopi_rate, random, freq;
	double a0, a1, b1, d; // for the low-pass filter
    Parameter *waveParam;
    int waveform, rate, random_count, period;
	
	// oscillator sync stuff
	int reset_offset, reset_cd, sync_c, sync_offset, sync, sync_period, reset_period;
	Oscillator *syncOsc;
	Parameter *syncParam;
	
    inline void doSine();
    inline void doSquare();
    inline void doSaw();
    inline void doNoise();
	inline void doRandom();
	inline float sqr(float foo);
	inline float saw(float foo);
};


#endif				/// _OSCILLATOR_H
