/* amSynth
 * (c) 2001,2002 Nick Dowell
 */
#ifndef _OSCILLATOR_H
#define _OSCILLATOR_H

#include "Synth--.h"
#include "../Parameter.h"

/**
 * @brief An Audio Oscillator unit.
 * 
 * Provides several different output waveforms (sine, saw, square, noise, 
 * random).
 */
class Oscillator:public NFSource, public FInput, public UpdateListener {
  public:
	Oscillator(int rate);
    virtual ~Oscillator();
    inline float *getNFData();
    void setInput(FSource & source);
	/**
	 * @param source The FSource which will control the puslewidth of the 
	 * waveform where appropriate (saw & square waves).
	 */
	void setPulseWidth(FSource & source);
    void setWaveform(int form);
	/**
	 * @param source The Parameter which will control the waveform of the
	 * oscillator.
	 */
    void setWaveform(Parameter & param);
	/* resets the oscillator.
	 * i.e. from the next call the oscillator will begin its cycle from the
	 * beginning.
	 */
	void reset();
	/*
	 * reset the oscillator, initially at sample indicated by offset, and then 
	 * every period samples. used for oscillator sync. 
	 * NB. period >= delta
	 */
    void reset( int offset, int period );
    void update();
	/**
	 * Sets the oscillator up to synchronise another oscillator (oscillator 
	 * sync).
	 * @param param This Parameter controls whether or not to synchronise the 
	 * other oscillator. 1 indicates yes, 0 no.
	 */
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
